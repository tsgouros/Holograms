///\file main.cpp
///\author Benjamin Knorlein
///\date 08/10/2016

#define NOMINMAX

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

#include <iostream>
#include <fstream>
#include "Socket.h"

#include <chrono>
#include <ctime>

#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
using namespace cv;

//processing options
//local
//bool online = false;
//std::string datafolder = "data";
//std::string filename = "testdata";

bool doPreloaded = false;
bool doRefine = true;
bool doWriteImages = false;
bool doGenerateDepthMaximum = false;
bool doMergebounds = true;
bool usePhase = true;
bool useAbs = false;

int maxAmplitude = 5.0;
double minAll = std::numeric_limits<double>::max();
double maxAll = std::numeric_limits<double>::min();

//remote
bool online = true;
#ifdef WIN32
std::string datafolder = "C:/holograms";
std::string ip = "10.12.160.99";
#else
std::string datafolder = "";
std::string ip = "172.20.160.24";
#endif

std::string filename = "flowcam_akashiwo_july24-0g-4us_24-Jul-2015_15-03-43-719.bmp";

std::string port = "1975";
Socket *sock = NULL;

//output
std::string outputfolder = "data_out";

//general settings
bool show = true;
int step_size = 100;
int min_depth = 1000;
int max_depth = 25000;
int width = 2048;
int height = 2048;

//max image
int windowsize = 5;

//contours
float max_threshold = 0.45;
double contour_minArea = 15.0;

float merge_threshold_depth = 400;
float merge_threshold_dist = 50;

int getIdxDepth(std::vector<int> &depths, int depth)
{
	int pos = std::find(depths.begin(), depths.end(), depth) - depths.begin();
	
	if (pos >= depths.size()) 
		return -1;
	
	return pos;
}

void writeImages(int start, int stop, int step_width)
{

	std::string outdir = outputfolder + "//" + filename;
	CreateDirectory(outdir.c_str(), NULL);

	sock = new Socket(ip, port);

	//set Image
	std::string hologram = datafolder + "/" + filename;
	std::cout << "Loading " << hologram << std::endl;
	sock->setImage(hologram);

	if (usePhase){
		sock->setOutputMode(2);
	}
	std::string name;
	std::vector<double> min_vals, max_vals;
	for (int d = start; d <= stop; d += step_width){
		std::cerr << "Load phase " << d << std::endl;
		float* data = new float[width * height];
		sock->receiveImageData(d, data);

		name = outdir + "//Phase_" + std::to_string(d) + ".ext";
		FILE* file = fopen(name.c_str(), "wb");
		fwrite(data, sizeof(float), width * height, file);
		fclose(file);

		cv::Mat image(cv::Size(width, height), CV_32FC1, data);

		double Min, Max;
		cv::minMaxLoc(image, &Min, &Max);
		min_vals.push_back(Min);
		max_vals.push_back(Max);

		cv::Mat image_disp;
		cv::Mat B;
		normalize(image, image_disp, 0, 255, CV_MINMAX);
		image_disp.convertTo(B, CV_8U);

		imwrite(outdir + "//Phase_" + std::to_string(d) + ".png", B);

		image.release();
		delete[] data;
	}

	name = outdir + "//Phase_MinMax.csv";
	std::ofstream ofs(name.c_str(), std::ofstream::out);
	for (int i = 0; i < min_vals.size(); i++)
	{
		ofs << min_vals[i] << " , " << max_vals[i] << std::endl;

	}
	ofs.close();

	if (usePhase){
		sock->setOutputMode(1);
	}

	min_vals.clear();
	max_vals.clear();

	for (int d = start; d <= stop; d += step_width){
		std::cerr << "Load amplitude " << d << std::endl;
		float* data = new float[width * height];
		sock->receiveImageData(d, data);

		name = outdir + "//Amplitude_" + std::to_string(d) + ".ext";
		FILE* file = fopen(name.c_str(), "wb");
		fwrite(data, sizeof(float), width * height, file);
		fclose(file);

		cv::Mat image(cv::Size(width, height), CV_32FC1, data);

		double Min, Max;
		cv::minMaxLoc(image, &Min, &Max);
		min_vals.push_back(Min);
		max_vals.push_back(Max);

		cv::Mat image_disp;
		cv::Mat B;
		normalize(image, image_disp, 0, 255, CV_MINMAX);
		image_disp.convertTo(B, CV_8U);

		imwrite(outdir + "//Amplitude_" + std::to_string(d) + ".png", B);

		image.release();
		delete[] data;
	}

	name = outdir + "//Amplitude_MinMax.csv";
	std::ofstream ofs2(name.c_str(), std::ofstream::out);
	for (int i = 0; i < min_vals.size(); i++)
	{
		ofs2 << min_vals[i] << " , " << max_vals[i] << std::endl;

	}
	ofs2.close();

	if (usePhase){
		sock->setOutputMode(1);
	}

	min_vals.clear();
	max_vals.clear();
	for (int d = start; d <= stop; d += step_width){
		std::cerr << "Load Intensity " << d << std::endl;
		float* data = new float[width * height];
		sock->receiveImageData(d, data);

		name = outdir + "//Intensity" + std::to_string(d) + ".ext";
		FILE* file = fopen(name.c_str(), "wb");
		fwrite(data, sizeof(float), width * height, file);
		fclose(file);

		cv::Mat image(cv::Size(width, height), CV_32FC1, data);

		double Min, Max;
		cv::minMaxLoc(image, &Min, &Max);
		min_vals.push_back(Min);
		max_vals.push_back(Max);

		cv::Mat image_disp;
		cv::Mat B;
		normalize(image, image_disp, 0, 255, CV_MINMAX);
		image_disp.convertTo(B, CV_8U);

		imwrite(outdir + "//Intensity" + std::to_string(d) + ".png", B);

		image.release();
		delete[] data;
	}

	name = outdir + "//Intensity_MinMax.csv";
	std::ofstream ofs3(name.c_str(), std::ofstream::out);
	for (int i = 0; i < min_vals.size(); i++)
	{
		ofs3 << min_vals[i] << " , " << max_vals[i] << std::endl;

	}
	ofs3.close();
}

void loadImages(std::vector<cv::Mat> &phase_images,
	std::vector<float *> &phase_images_data,
	std::vector<int> &depths, int start, int stop, int step_width, bool skip_load = false)
{
	for (int d = start; d <= stop; d += step_width){
		
		if (getIdxDepth(depths, d) < 0){
			std::cout << "Loading " << d << std::endl;

			float* data = new float[width * height];

			if (!skip_load && online)
			{
				sock->receiveImageData(d, data);

			}
			else
			{
				std::string name = datafolder + "//Phase_" + std::to_string(d) + ".ext";
				FILE* file = fopen(name.c_str(), "rb");
				fread(data, sizeof(float), width * height, file);
				fclose(file);
			}

			cv::Mat image(cv::Size(width, height), CV_32FC1, data);
			if (useAbs) image = abs(image);

			double Min, Max;
			cv::minMaxLoc(image, &Min, &Max);
			minAll = (minAll > Min) ? Min : minAll;
			maxAll = (maxAll < Max) ? Max : maxAll;

			//remove borders:
			int size = 100;
			if (d < 3000){
				size = 300;
			}
			else if (d < 7000)
			{
				size = 150;
			}


			float val = (!usePhase) ? Max : Min;

			if (size > 0)
			{		
				image(cv::Rect(0, 0, size, image.rows)) = val;
				image(cv::Rect(0, 0, image.cols, size)) = val;
				image(cv::Rect(image.cols - size, 0, size, image.rows)) = val;
				image(cv::Rect(0, image.rows - size, image.cols, size)) = val;
			}

			if (show)
			{
				cv::Mat image_disp;
				cv::Mat B;
				normalize(image, image_disp, 0, 255, CV_MINMAX);
				image_disp.convertTo(B, CV_8U);
				imshow("Loading", B);
				cv::waitKey(1);
				//imwrite("img_" + std::to_string(d) + ".jpg", B);
			}

			phase_images.push_back(image);
			phase_images_data.push_back(data);
			depths.push_back(d);
		} 
		else
		{
			std::cout << "Skip Load " << d << std::endl;
		}
	}

	if (show)
	{
		destroyWindow("Loading");
	}
}

cv::Mat findMaximas(std::vector<cv::Mat> phase_images,
					std::vector<int> depths, int start, int stop, int step_width, cv::Mat &maxDepths)
{
	float val = (usePhase) ? minAll : maxAll;
	cv::Mat image_maximum(cv::Size(width, height), CV_32FC1, cvScalar(val));

	if (show)
	{
		imshow("Maximum", 0);
	}

	for (int d = start; d <= stop; d += step_width){
		std::cout << "Maximum " << d << std::endl;

		int idx = getIdxDepth(depths, d);

		//filter image
		Mat kernel = cv::Mat(windowsize, windowsize, CV_32FC1, Scalar::all(1.0 / (windowsize*windowsize)));
		cv::Mat image_tmp;
		filter2D(phase_images[idx], image_tmp, CV_32F, kernel);

		//compute max
		if (!doGenerateDepthMaximum){
			if (usePhase){
				image_maximum = cv::max(image_maximum, image_tmp);
			} else
			{
				image_maximum = cv::min(image_maximum, image_tmp);
			}
		} 
		else
		{
			float* max_ptr = (float *) image_maximum.data;
			float* image_ptr = (float *) image_tmp.data;
			float * depth_ptr = (float *) maxDepths.data;

			for (int i = 0; i < width * height; i++, max_ptr++, image_ptr++, depth_ptr++)
			{
				if ((usePhase && *max_ptr < *image_ptr && *image_ptr) ||
					!usePhase && *max_ptr > *image_ptr && *image_ptr){
					*max_ptr = *image_ptr;
					*depth_ptr = (float)d;
				}
			}
		
		}

		if (show)
		{
			cv::Mat image_disp;
			cv::Mat B;
			normalize(image_maximum, image_disp, 0, 255, CV_MINMAX);
			image_disp.convertTo(B, CV_8U);
			imshow("Maximum", B);
			cv::waitKey(1);
		}
	}

	if (show)
	{
		destroyWindow("Maximum");
	}

	return image_maximum;
}

void findContours(std::string outdir, cv::Mat image_maximum, cv::Mat image_maximumDepth, std::vector<std::vector<Point> > &contours, std::vector<cv::Rect> &bounds)
{

	normalize(image_maximum, image_maximum, 0.0, 1.0, CV_MINMAX);
	
	//threshold
	if (usePhase){
		threshold(image_maximum, image_maximum, 1.0 - max_threshold, 1., CV_THRESH_BINARY);
	} 
	else
	{
		threshold(image_maximum, image_maximum, max_threshold, 1., CV_THRESH_BINARY_INV);
	}

	//convert to 8U
	cv::Mat bw;
	image_maximum.convertTo(bw, CV_8U);

	if (doGenerateDepthMaximum){
		image_maximumDepth = image_maximumDepth.mul(image_maximum);

		if (show)
		{
			std::vector<Vec3b> colors;
			for (size_t i = 0; i < 255; i++)
			{
				int b = theRNG().uniform(0, 255);
				int g = theRNG().uniform(0, 255);
				int r = theRNG().uniform(0, 255);
				colors.push_back(Vec3b((uchar)b, (uchar)g, (uchar)r));
			}

			Mat dst = Mat::zeros(image_maximumDepth.size(), CV_8UC3);
			// Fill labeled objects with random colors
			for (int i = 0; i < dst.rows; i++)
			{
				for (int j = 0; j < dst.cols; j++)
				{
					int index = image_maximumDepth.at<float>(i, j) / 100;
					dst.at<Vec3b>(i, j) = colors[index];
				}
			}

			imshow("Depths", dst);
			cvWaitKey(0);
			destroyWindow("Depths");
			imwrite(outdir + "//depth.png", dst);
		}
	}

	// Find total markers
	std::vector<std::vector<Point> > contours_org;
	findContours(bw, contours_org, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	//filter by Area and position
	for (size_t i = 0; i < contours_org.size(); i++)
	{
		cv::Moments mu = moments(contours_org[i], false);
		cv::Point2f center = cv::Point2f(mu.m10 / mu.m00, mu.m01 / mu.m00);
		if (cv::contourArea(contours_org[i]) >= contour_minArea)
		{
			contours.push_back(contours_org[i]);
			cv::Rect boundingBox = boundingRect(contours_org[i]);
			bounds.push_back(boundingBox);
		}
	}
	std::cout << " Contours found " << contours.size() << std::endl;

	RNG rng(12345);
	Mat drawing = Mat::zeros(bw.size(), CV_8UC3);
	for (int i = 0; i< contours.size(); i++)
	{
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		drawContours(drawing, contours, i, color, -1, 8, 0, 0, Point());
		rectangle(drawing, bounds[i].tl(), bounds[i].br(), color, 2, 8, 0);
		putText(drawing, std::to_string(static_cast<long long>(i)), bounds[i].br(),
			FONT_HERSHEY_COMPLEX_SMALL, 2, color, 1, CV_AA);
	}

	imwrite(outdir + "//contours" + ".png", drawing);

	if (show){
		imshow("Contours", drawing);
		cvWaitKey(1);
		destroyWindow("Contours");
	}
}

void setContourDepth(std::vector<cv::Mat> phase_images, std::vector<int> depths, std::vector<cv::Rect> bounds, std::vector<int> &depths_contour, std::vector<double> &vals_contour
	, int start, int stop, int step_width, int id = -1)
{
	int start_c = (id == -1) ? 0 : id;
	int stop_c = (id == -1) ? bounds.size() - 1 : id;

	for (int c = start_c; c <= stop_c; c++)
	{
		std::cout << "Contour " << c << std::endl;
		int best_depth = 0;
		float maxVal = (usePhase) ? minAll : maxAll;

		for (int d = start; d <= stop; d += step_width){
			int idx = getIdxDepth(depths, d);

			cv::Mat roi = phase_images[idx](bounds[c]);
			float val = sum(roi)[0] / bounds[c].area();
			if ((usePhase && val > maxVal) || 
				(!usePhase && val < maxVal))
			{
				maxVal = val;
				best_depth = depths[idx];
			}
		}
		depths_contour[c] = best_depth;
		vals_contour[c] = maxVal;
		std::cout << "Best Depth " << best_depth << std::endl;
	}
}

void saveROI(std::string outdir, std::vector<cv::Rect> bounds, std::vector<int> depths_contour, int mode)
{	
	for (size_t c = 0; c < bounds.size(); c++)
	{
	  std::cout << "Save Contour " << c << " at depth " << std::to_string(static_cast<long long>(depths_contour[c])) << std::endl;

		float* data = new float[width * height];

		if (online)
		{
			//somehow we have to request the image twice
			sock->receiveImageData(depths_contour[c], data);
		}
		else
		{
		  std::string name = "d:\\data//Amplitude_" + std::to_string(static_cast<long long>(depths_contour[c])) + ".ext";
			//std::string name = datafolder + "//Amplitude_" + std::to_string(depths_contour[c]) + ".ext";

			FILE* file = fopen(name.c_str(), "rb");
			fread(data, sizeof(float), width * height, file);
			fclose(file);
		}

		cv::Mat image(cv::Size(width, height), CV_32FC1, data);
		if (mode == 2 && useAbs) image = abs(image);

		cv::Rect bound_cont = bounds[c];
		bound_cont.x = bound_cont.x - 20;
		bound_cont.y = bound_cont.y - 20;
		bound_cont.width = bound_cont.width + 40;
		bound_cont.height = bound_cont.height + 40;

		if (bound_cont.x < 0) bound_cont.x = 0;
		if (bound_cont.y < 0) bound_cont.y = 0;
		if (bound_cont.y + bound_cont.height > image.rows) bound_cont.height = image.rows - bound_cont.y;
		if (bound_cont.x + bound_cont.width > image.cols) bound_cont.width = image.cols - bound_cont.x;

		cv::Mat roi = image(bound_cont);

		cv::Mat image_display;
		cv::Mat drawing;
		normalize(roi, image_display, 0, 255, CV_MINMAX);
		image_display.convertTo(drawing, CV_8U);

		if (mode == 1){
		  imwrite(outdir + "//contours_" + std::to_string(static_cast<long long>(c)) + ".png", drawing);
		} else if (mode == 2){
		  imwrite(outdir + "//contoursPhase_" + std::to_string(static_cast<long long>(c)) + ".png", drawing);
		}

		if (show)
		{
			imshow("ROI", drawing);
			cv::waitKey(1);
			destroyWindow("ROI");
		}

		image.release();
		delete[] data;
	}
}

void writeReport(std::string outdir, std::vector<cv::Rect> bounds, std::vector<int> depths_contour, std::vector<double> vals_contour, double time)
{

	std::ifstream infile;
	infile.open("template.xml", std::ifstream::in);

	char * buffer;
	long size;

	// get size of file
	infile.seekg(0);
	std::streampos fsize = 0;
	infile.seekg(0, std::ios::end);
	size = infile.tellg() - fsize;
	infile.seekg(0);

	// allocate memory for file content
	buffer = new char[size];

	// read content of infile
	infile.read(buffer, size);

	infile.close();

	std::ofstream outfile(outdir + "//report.xml", std::ofstream::out);
	// write to outfile
	std::string buffer_st(buffer);
	outfile.write(buffer, size);
	delete[] buffer;

	outfile << "<DATA>" << std::endl;
	outfile << "<FILENAME>" << filename << "</FILENAME>" << std::endl;
	outfile << "<CONTOURIMAGE>contours.png</CONTOURIMAGE>" << std::endl;
	outfile << "<MAXIMAGE>maximum.png</MAXIMAGE>" << std::endl;
	outfile << "<TIME>" << time << "</TIME>" << std::endl;

	for (size_t c = 0; c < bounds.size(); c++)
	{
		outfile << "<ROI>" << std::endl;
		outfile << "<CONTOUR>" << c << "</CONTOUR>" << std::endl;
		outfile << "<X>" << (bounds[c].tl() + bounds[c].br()).x * 0.5 << "</X>" << std::endl;
		outfile << "<Y>" << (bounds[c].tl() + bounds[c].br()).y * 0.5 << "</Y>" << std::endl;
		outfile << "<WIDTH>" << bounds[c].width << "</WIDTH>" << std::endl;
		outfile << "<HEIGHT>" << bounds[c].height << "</HEIGHT>" << std::endl;
		outfile << "<DEPTH>" << depths_contour[c] << "</DEPTH>" << std::endl;
		outfile << "<VAL>" << vals_contour[c] << "</VAL>" << std::endl;
		outfile << "<IMAGE>" << "contours_" + std::to_string(static_cast<long long>(c)) + ".png" << "</IMAGE>" << std::endl;
		outfile << "<IMAGEPHASE>" << "contoursPhase_" + std::to_string(static_cast<long long>(c)) + ".png" << "</IMAGEPHASE>" << std::endl;
		outfile << "</ROI>" << std::endl;
	}
	outfile << "</DATA>" << std::endl;
	outfile << "</doc>" << std::endl;

	outfile.close();
}

bool DoBoxesIntersect(cv::Rect a, cv::Rect b) {
	if (a.x + a.width + merge_threshold_dist< b.x) return false; // a is left of b
	if (a.x > b.x + b.width + merge_threshold_dist) return false; // a is right of b
	if (a.y + a.height + merge_threshold_dist < b.y) return false; // a is above b
	if (a.y > b.y + b.height + merge_threshold_dist) return false; // a is below b
	return true; // boxes overlap
}

bool mergebounds(std::vector<cv::Rect> &bounds, std::vector<int> &depths_contour, std::vector<double> &vals_contour)
{
	std::cerr << "Test Intersect" << std::endl;

	bool merged = false;
	for (int c = 0; c < bounds.size(); c++)
	{
		for (int k = c+1; k < bounds.size(); k++)
		{
			if (DoBoxesIntersect(bounds[c], bounds[k]) && abs(depths_contour[c] - depths_contour[k]) <= merge_threshold_depth)
			{
				std::cerr << "Intersect " << c << " " << k << std::endl;
				int x_max = max(bounds[c].x + bounds[c].width, bounds[k].x + bounds[k].width);
				int y_max = max(bounds[c].y + bounds[c].height, bounds[k].y + bounds[k].height);

				bounds[c].x = min(bounds[c].x, bounds[k].x);
				bounds[c].y = min(bounds[c].y, bounds[k].y);
				bounds[c].width = x_max - bounds[c].x;
				bounds[c].height = y_max - bounds[c].y;

				depths_contour[c] = (depths_contour[c] + depths_contour[k]) * 0.5;
				vals_contour[c] = (vals_contour[c] + vals_contour[k]) * 0.5;
				bounds.erase(bounds.begin() + k);
				depths_contour.erase(depths_contour.begin() + k);
				vals_contour.erase(vals_contour.begin() + k);
				k = k - 1;
				merged = true;
			}
		}
	}
	return merged;
}

int main(int argc, char** argv)
{

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	if (argc > 1)
	{
		filename = std::string(argv[1]);
	}

	if (doWriteImages)
	{
		writeImages(min_depth, max_depth, step_size);
		return 1;
	}

	std::string outdir = outputfolder + "//" + filename;

#ifdef _MSC_VER
	CreateDirectory(outdir.c_str(), NULL);
#else
	mkdir(outdir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

	//establish communication
	if (online)
	{
		//connect
		sock = new Socket(ip, port);

		//set Image
		std::string hologram = datafolder + "/" + filename;
		std::cout << "Loading " << hologram << std::endl;
		sock->setImage(hologram);

		if (online)
		{
			if (usePhase){
				sock->setOutputMode(2);
			} 
			else
			{
				sock->setOutputMode(1);
			}
		}
	}

	std::vector<cv::Mat> phase_images;
	std::vector<float *> phase_images_data;
	std::vector<int> depths;

	if (show)
	{
		imshow("Loading", 0);
	}

	////////Load Data
	loadImages(phase_images, phase_images_data, depths, min_depth, max_depth, step_size, doPreloaded);

	cv::Mat image_maximumDepth(cv::Size(width, height), CV_32FC1, cvScalar(0.));


	////////Find Maximas
	cv::Mat image_maximum = findMaximas(phase_images, depths, min_depth, max_depth, step_size, image_maximumDepth);
	{
		std::string name = outdir + "//maximum.ext";
		FILE* file = fopen(name.c_str(), "wb");
		fwrite(image_maximum.data, sizeof(float), width * height, file);
		fclose(file);

		name = outdir + "//maximumDepth.ext";
		file = fopen(name.c_str(), "wb");
		fwrite(image_maximumDepth.data, sizeof(float), width * height, file);
		fclose(file);
	}
	if (show)
	{
		cv::Mat image_disp;
		cv::Mat B;
		normalize(image_maximum, image_disp, 0, 255, CV_MINMAX);
		image_disp.convertTo(B, CV_8U);

		imwrite(outdir + "//maximum.png", B);
	}



////////Find contours
	std::vector<std::vector<Point> > contours;
	std::vector<cv::Rect> bounds;
	
	findContours(outdir, image_maximum, image_maximumDepth, contours, bounds);

////////Split Bounds

////////Find best depth for contour
	std::vector<int> depths_contour;
	std::vector<double> val_contour;

	depths_contour.resize(contours.size());
	val_contour.resize(contours.size());
	setContourDepth(phase_images, depths, bounds, depths_contour, val_contour, min_depth, max_depth, step_size);

//merge bounds
	if (doMergebounds){
		while (mergebounds(bounds, depths_contour, val_contour));

		RNG rng(12345);
		Mat drawing = Mat::zeros(cv::Size(width, height), CV_8UC3);
		for (int i = 0; i< contours.size(); i++)
		{
			Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
			drawContours(drawing, contours, i, color, 2, 8, 0, 0, Point());
		}

		for (int i = 0; i< bounds.size(); i++)
		{
			Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
			rectangle(drawing, bounds[i].tl(), bounds[i].br(), color, 2, 8, 0);
			putText(drawing, std::to_string(static_cast<long long>(i)), bounds[i].br(),
				FONT_HERSHEY_COMPLEX_SMALL, 2, color, 1, CV_AA);
		}
		
		imwrite(outdir + "//contours" + ".png", drawing);

		if (show){
			imshow("Contours", drawing);
			cvWaitKey(1);
			destroyWindow("Contours");
		}
	}

///////Refine depths
	if (doRefine && online)
	{ 
		for (size_t c = 0; c < bounds.size(); c++)
		{
			//step 1
			loadImages(phase_images, phase_images_data, depths, depths_contour[c] - step_size, depths_contour[c] + step_size, step_size / 10);

			setContourDepth(phase_images, depths, bounds, depths_contour, val_contour, depths_contour[c] - step_size, depths_contour[c] + step_size, step_size / 10, c);
			
		}
	}
////////Save ROIs
	if (online)
	{
		if (!usePhase){
			sock->setOutputMode(2);
		}
	}
	saveROI(outdir, bounds, depths_contour,2);

	if (online)
	{
		sock->setOutputMode(1);
	}
	saveROI(outdir, bounds, depths_contour,1);

////////Create Report
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

	writeReport(outdir, bounds, depths_contour, val_contour, std::chrono::duration_cast<std::chrono::minutes>(end - begin).count());
	
////////Cleanup

	if (online && sock) delete sock;

	for (int i = 0; i < phase_images.size() ; i ++){
		phase_images[i].release();
		delete phase_images_data[i];
	}

	return 0;
}

