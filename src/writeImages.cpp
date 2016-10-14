///\file main.cpp
///\author Benjamin Knorlein
///\date 08/10/2016

#include "OctopusClient.h"
#include "Settings.h"
#include <iostream>

#ifdef _MSC_VER
	#define _CRT_SECURE_NO_WARNINGS

	#ifndef WITH_CONSOLE
		#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
	#endif

	#include <windows.h>
#else
	#include <sys/stat.h>
	#include <sys/types.h>
#endif

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

std::string slash = "/"; 

void writeImages(std::string filename, Settings * settings)
{
	int width = settings->getWidth();
	int height = settings->getHeight();
	std::string datafolder = settings->getDatafolder();
	int start = settings->getMinDepth();
	int stop = settings->getMaxDepth();
	int step_width = settings->getStepSize();

	std::string outFile = settings->getOutputFolder() + slash + filename;
#ifdef _MSC_VER
	CreateDirectory(settings->getOutputFolder().c_str(), NULL);
	CreateDirectory(outFile.c_str(), NULL);
#else
	mkdir(outputFolder.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

	OctopusClient * client = new OctopusClient(settings->getIp(), settings->getPort());

	std::cout << "Loading " << datafolder + "/" + filename  << " with Background" << std::endl;
	if (!client->setSourceHologram(datafolder + "/", filename , "background.bmp"))exit;

	std::string name;
	for (int d = start; d <= stop; d += step_width){
		std::cerr << "Save amplitude " << d << " with Background" << std::endl;
		float* data = new float[width * height];
		client->getAmplitudeImage(d, data);

		name = outFile + slash + "AmplitudeB_" + std::to_string(d) + ".ext";
		FILE* file = fopen(name.c_str(), "wb");
		fwrite(data, sizeof(float), width * height, file);
		fclose(file);

		cv::Mat image(cv::Size(width, height), CV_32FC1, data);

		cv::Mat image_disp;
		cv::Mat B;
		normalize(image, image_disp, 0, 255, CV_MINMAX);
		image_disp.convertTo(B, CV_8U);

		imwrite(outFile + slash + "AmplitudeB_" + std::to_string(d) + ".png", B);

		image.release();
		delete[] data;
	}

	for (int d = start; d <= stop; d += step_width){
		std::cerr << "Save amplitude " << d << " with Background" << std::endl;
		float* data = new float[width * height];
		client->getIntensityImage(d, data);

		name = outFile + slash + "IntensityB_" + std::to_string(d) + ".ext";
		FILE* file = fopen(name.c_str(), "wb");
		fwrite(data, sizeof(float), width * height, file);
		fclose(file);

		cv::Mat image(cv::Size(width, height), CV_32FC1, data);

		cv::Mat image_disp;
		cv::Mat B;
		normalize(image, image_disp, 0, 255, CV_MINMAX);
		image_disp.convertTo(B, CV_8U);

		imwrite(outFile + slash + "IntensityB_" + std::to_string(d) + ".png", B);

		image.release();
		delete[] data;
	}

	for (int d = start; d <= stop; d += step_width){	
		std::cerr << "Save Phase " << d << " with Background" << std::endl;
		if (!client->setSourceHologram(datafolder + "/", filename))exit;

		float* data1 = new float[width * height];
		client->getPhaseImage(d, data1);

		if (!client->setSourceHologram(datafolder + "/", "background.bmp"))exit;
		float* data2 = new float[width * height];
		client->getPhaseImage(d, data2);

		float* data3 = new float[width * height];

		float* dt1_ptr = data1;
		float* dt2_ptr = data2;
		float* dt3_ptr = data3;

		for (int i = 0; i < width*height; i++, ++dt1_ptr, ++dt2_ptr, ++dt3_ptr)
		{
			*dt3_ptr = *dt1_ptr - *dt2_ptr;
		}

		name = outFile + slash + "PhaseB_" + std::to_string(d) + ".ext";
		FILE* file = fopen(name.c_str(), "wb");
		fwrite(data3, sizeof(float), width * height, file);
		fclose(file);

		//cv::Mat image1(cv::Size(width, height), CV_32FC1, data1);
		//cv::Mat image2(cv::Size(width, height), CV_32FC1, data2);
		cv::Mat image3(cv::Size(width, height), CV_32FC1, data3);
		cv::Mat image_disp;
		cv::Mat B;
		//normalize(image1, image_disp, 0, 255, CV_MINMAX);
		//image_disp.convertTo(B, CV_8U);
		//imwrite(outputFolder + slash + "Phase1B_" + std::to_string(d) + ".png", B);
		//normalize(image2, image_disp, 0, 255, CV_MINMAX);
		//image_disp.convertTo(B, CV_8U);
		//imwrite(outputFolder + slash + "Phase2B_" + std::to_string(d) + ".png", B);
		normalize(image3, image_disp, 0, 255, CV_MINMAX);
		image_disp.convertTo(B, CV_8U);
		imwrite(outFile + slash + "PhaseB_" + std::to_string(d) + ".png", B);

		//image1.release();
		//image2.release();
		image3.release();
		delete[] data1;
		delete[] data2;
		delete[] data3;
	}

	//set Image
	std::cout << "Loading " << datafolder + "/" + filename << std::endl;
	if (!client->setSourceHologram(datafolder + "/", filename))exit;

	for (int d = start; d <= stop; d += step_width){
		std::cerr << "Save phase " << d << std::endl;
		float* data = new float[width * height];
		client->getPhaseImage(d, data);

		name = outFile + slash + "Phase_" + std::to_string(d) + ".ext";
		FILE* file = fopen(name.c_str(), "wb");

		fwrite(data, sizeof(float), width * height, file);
		fclose(file);

		cv::Mat image(cv::Size(width, height), CV_32FC1, data);

		cv::Mat image_disp;
		cv::Mat B;
		normalize(image, image_disp, 0, 255, CV_MINMAX);
		image_disp.convertTo(B, CV_8U);

		imwrite(outFile + slash + "Phase_" + std::to_string(d) + ".png", B);

		image.release();
		delete[] data;
	}

	for (int d = start; d <= stop; d += step_width){
		std::cerr << "Save amplitude " << d << std::endl;
		float* data = new float[width * height];
		client->getAmplitudeImage(d, data);

		name = outFile + slash + "Amplitude_" + std::to_string(d) + ".ext";
		FILE* file = fopen(name.c_str(), "wb");
		fwrite(data, sizeof(float), width * height, file);
		fclose(file);

		cv::Mat image(cv::Size(width, height), CV_32FC1, data);

		cv::Mat image_disp;
		cv::Mat B;
		normalize(image, image_disp, 0, 255, CV_MINMAX);
		image_disp.convertTo(B, CV_8U);

		imwrite(outFile + slash + "Amplitude_" + std::to_string(d) + ".png", B);

		image.release();
		delete[] data;
	}


	for (int d = start; d <= stop; d += step_width){
		std::cerr << "Save Intensity " << d << std::endl;
		float* data = new float[width * height];
		client->getIntensityImage(d, data);

		name = outFile + slash + "Intensity_" + std::to_string(d) + ".ext";
		FILE* file = fopen(name.c_str(), "wb");
		fwrite(data, sizeof(float), width * height, file);
		fclose(file);

		cv::Mat image(cv::Size(width, height), CV_32FC1, data);

		cv::Mat image_disp;
		cv::Mat B;
		normalize(image, image_disp, 0, 255, CV_MINMAX);
		image_disp.convertTo(B, CV_8U);

		imwrite(outFile + slash + "Intensity_" + std::to_string(d) + ".png", B);

		image.release();
		delete[] data;
	}
}

int main(int argc, char** argv)
{

	if (argc < 3)
	{
		std::cerr << "You need to provide a filename and a settings.xml" << std::endl;
		return 0;
	}

	std::string filename = std::string(argv[1]);
	Settings * settings = new Settings(argv[2]);

	writeImages(filename, settings);

	delete settings;
	return 1;
}

