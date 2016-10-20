///\file main.cpp
///\author Benjamin Knorlein
///\date 08/10/2016

#define NOMINMAX

#include "ImageCache.h"
#include "OctopusClient.h"
#include "OfflineReader.h"
#include "ReportWriter.h"
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

#include <chrono>
#include <ctime>

#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

std::string slash = "/"; 

double minAll = std::numeric_limits<double>::max();
double maxAll = std::numeric_limits<double>::min();
//
//void loadImages(ImageCache* cache, int start, int stop, int step_width)
//{
//	if (show)
//	{
//
//		cv::namedWindow("Loading", cv::WINDOW_NORMAL);
//		cv::resizeWindow("Loading", 800, 800);
//		imshow("Loading", 0);
//	}
//
//	for (int d = start; d <= stop; d += step_width){		
//		cv::Mat* image = cache->getPhaseImage(d);
//		if (useAbs) *image = cv::abs(*image);
//
//		//remove borders:
//		int size = 100;
//		if (d < 3000){
//			size = 300;
//		}
//		else if (d < 7000)
//		{
//			size = 150;
//		}
//
//		float val = 0;
//
//		if (size > 0)
//		{		
//			(*image)(cv::Rect(0, 0, size, image->rows)) = val;
//			(*image)(cv::Rect(0, 0, image->cols, size)) = val;
//			(*image)(cv::Rect(image->cols - size, 0, size, image->rows)) = val;
//			(*image)(cv::Rect(0, image->rows - size, image->cols, size)) = val;
//		}
//
//		if (show)
//		{
//			cv::Mat image_disp;
//			cv::Mat B;
//			cv::normalize((*image), image_disp, 0, 255, CV_MINMAX);
//			image_disp.convertTo(B, CV_8U);
//			imshow("Loading", image_disp);
//			cv::waitKey(1);
//		} 
//	}
//
//	if (show)
//	{
//		cv::destroyWindow("Loading");
//	}
//}

cv::Mat findMaximas(ImageCache* cache, int start, int stop, int step_width, int width, int height, int windowsize, bool show)
{

	cv::Mat image_maximum(cv::Size(width, height), CV_32FC1, cvScalar(0));

	if (show)
	{
		cv::namedWindow("Maximum", cv::WINDOW_NORMAL);
		cv::resizeWindow("Maximum", 800, 800);

		imshow("Maximum", 0);
	}

	for (int d = start; d <= stop; d += step_width){
		std::cout << "Maximum " << d << std::endl;

		//filter image
		Mat kernel = cv::Mat(windowsize, windowsize, CV_32FC1, Scalar::all(1.0 / (windowsize*windowsize)));
		cv::Mat image_tmp;
		filter2D(*cache->getPhaseImage(d), image_tmp, CV_32F, kernel);


		float* max_ptr = (float *) image_maximum.data;
		float* image_ptr = (float *) image_tmp.data;

		for (int i = 0; i < width * height; i++, max_ptr++, image_ptr++)
		{
			if ( *max_ptr > *image_ptr && *image_ptr)
			{
				*max_ptr = *image_ptr;
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

cv::Mat findContours(std::string outdir, cv::Mat image_maximum, std::vector<std::vector<Point> > &contours, std::vector<cv::Rect> &bounds, double max_threshold, double contour_minArea, bool show)
{

	normalize(image_maximum, image_maximum, 0.0, 1.0, CV_MINMAX);
	
	//threshold	
	threshold(image_maximum, image_maximum, 1.0 - max_threshold, 1., CV_THRESH_BINARY);
	
	//convert to 8U
	cv::Mat bw;
	image_maximum.convertTo(bw, CV_8U);

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
		putText(drawing, std::to_string(((long long)i)), bounds[i].br(),
			FONT_HERSHEY_COMPLEX_SMALL, 2, color, 1, CV_AA);
	}

	if (show){
		imshow("Contours", drawing);
		cvWaitKey(1);
		destroyWindow("Contours");
	}

	return drawing;
}

void setContourDepth(ImageCache* cache, std::vector<cv::Rect> bounds, std::vector<int> &depths_contour, std::vector<double> &vals_contour
	, int start, int stop, int step_width, int id = -1)
{
	int start_c = (id == -1) ? 0 : id;
	int stop_c = (id == -1) ? bounds.size() - 1 : id;

	for (int c = start_c; c <= stop_c; c++)
	{
		std::cout << "Contour " << c << std::endl;
		int best_depth = 0;
		float maxVal = minAll;

		for (int d = start; d <= stop; d += step_width){
			cv::Mat * image = cache->getPhaseImage(d);

			cv::Mat roi = (*image)(bounds[c]);

			float val = sum(roi)[0] / bounds[c].area();
			if (val > maxVal)
			{
				maxVal = val;
				best_depth = d;
			}
		}
		depths_contour[c] = best_depth;
		vals_contour[c] = maxVal;
		std::cout << "Best Depth " << best_depth << std::endl;
	}
}

bool DoBoxesIntersect(int merge_threshold_dist, cv::Rect a, cv::Rect b) {
	if (a.x + a.width + merge_threshold_dist< b.x) return false; // a is left of b
	if (a.x > b.x + b.width + merge_threshold_dist) return false; // a is right of b
	if (a.y + a.height + merge_threshold_dist < b.y) return false; // a is above b
	if (a.y > b.y + b.height + merge_threshold_dist) return false; // a is below b
	return true; // boxes overlap
}

bool mergebounds(std::vector<cv::Rect> &bounds, std::vector<int> &depths_contour, std::vector<double> &vals_contour, int merge_threshold_depth, int merge_threshold_dist)
{
	std::cerr << "Test Intersect" << std::endl;

	bool merged = false;
	for (int c = 0; c < bounds.size(); c++)
	{
		for (int k = c+1; k < bounds.size(); k++)
		{
			if (DoBoxesIntersect(merge_threshold_dist, bounds[c], bounds[k]) && abs(depths_contour[c] - depths_contour[k]) <= merge_threshold_depth)
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

	if (argc < 3)
	{
		std::cerr << "You need to provide a filename and a settings.xml" << std::endl;
		return 0;
	}

	std::string filename = std::string(argv[1]);
	Settings * settings = new Settings(argv[2]);

	std::string outFile = settings->getOutputFolder() + slash + filename;

#ifdef _MSC_VER
	CreateDirectory(outFile.c_str(), NULL);
#else
	mkdir(settings->getOutputFolder().c_str(), 
	      S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

	//setup ReportWriter
	ReportWriter * writer = new ReportWriter(settings->getOutputFolder(), filename);
	//setup ImageCache
	ImageCache * cache;
	if (settings->getOnline())
	{
		cache = new ImageCache(new OctopusClient(settings->getIp(), settings->getPort()), 500);
	} 
	else
	{
		cache = new ImageCache(new OfflineReader(), 500);

	}
	cache->getImageSource()->setSourceHologram(settings->getDatafolder(), filename);

	////////Load Data
	//loadImages(cache, min_depth, max_depth, step_size);

	////////Find Maximas
	cv::Mat image_maximum = findMaximas(cache, settings->getMinDepth(), settings->getMaxDepth(), settings->getStepSize(),settings->getWidth(), settings->getHeight(),settings->getWindowsize(), settings->getShow());
	
	writer->saveImage(image_maximum, "maximum.png", true);

////////Find contours
	std::vector<std::vector<Point> > contours;
	std::vector<cv::Rect> bounds;
	
	cv::Mat contourImage = findContours(settings->getOutputFolder(), image_maximum, contours, bounds, settings->getMaxThreshold(), settings->getContourMinArea(), settings->getShow());
	writer->saveImage(contourImage, "contours.png");

////////Find best depth for contour
	std::vector<int> depths_contour;
	std::vector<double> val_contour;

	depths_contour.resize(contours.size());
	val_contour.resize(contours.size());
	setContourDepth(cache, bounds, depths_contour, val_contour, settings->getMinDepth(), settings->getMaxDepth(), settings->getStepSize());

//merge bounds
	if (settings->getDoMergebounds()){
		while (mergebounds(bounds, depths_contour, val_contour, settings->getMergeThresholdDepth(), settings->getMergeThresholdDist()));

		RNG rng(12345);
		Mat drawing = Mat::zeros(cv::Size(settings->getWidth(), settings->getHeight()), CV_8UC3);
		for (int i = 0; i< contours.size(); i++)
		{
			Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
			drawContours(drawing, contours, i, color, 2, 8, 0, 0, Point());
		}

		for (int i = 0; i< bounds.size(); i++)
		{
			Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
			rectangle(drawing, bounds[i].tl(), bounds[i].br(), color, 2, 8, 0);
			putText(drawing, std::to_string(((long long)i)), bounds[i].br(),
				FONT_HERSHEY_COMPLEX_SMALL, 2, color, 1, CV_AA);
		}
		
		imwrite(settings->getOutputFolder() + slash + "contours" + ".png", drawing);

		if (settings->getShow()){
			imshow("Contours", drawing);
			cvWaitKey(1);
			destroyWindow("Contours");
		}
	}

///////Refine depths
	if (settings->getDoRefine() && settings->getOnline())
	{ 
		for (size_t c = 0; c < bounds.size(); c++)
		{
			setContourDepth(cache, bounds, depths_contour, val_contour, depths_contour[c] - settings->getStepSize(), depths_contour[c] + settings->getStepSize(), settings->getStepSize() / 10, c);
		}
	}

////////Create Report
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	writer->writeXMLReport(bounds, depths_contour, val_contour, std::chrono::duration_cast<std::chrono::minutes>(end - begin).count());
	writer->saveROIImages(cache, bounds, depths_contour);

////////Cleanup
	delete cache->getImageSource();
	delete cache;
	delete writer;
	return 0;
}

