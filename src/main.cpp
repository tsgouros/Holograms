///\file main.cpp
///\author Benjamin Knorlein
///\date 08/10/2016

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef _MSC_VER 
#ifndef WITH_CONSOLE
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif
#endif

#include <iostream>
#include "Socket.h"

#include <opencv2/core.hpp>
#include "opencv2/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
using namespace cv;

RNG rng(12345);

//processing settings
bool show = true;
int step_start = 2;
int step_end = 2;
std::string datafolder = "data";

//general settings
int step_size = 100;
int min_depth = 1000;
int max_depth = 20000;
int width = 2048;
int height = 2048;

//step 2 options
double contour_minArea = 15.0;
double border_minDist = 250.0;

//saves the Data
void step1()
{
	Socket *sock = new Socket("10.12.160.99", "1975");
	Sleep(500);

	if (show) cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE);// Create a window for display.

	sock->sendMessage("SET_API_VERSION 2\n");
	Sleep(500);

	for (int output = 0; output < 3; output++){
		sock->sendMessage("OUTPUT_MODE " + std::to_string(output) + "\n0\n");
		Sleep(500);
		std::string reply;
		while (reply.empty()){
			reply = sock->receiveMessage();
		}

		for (int d = min_depth; d <= max_depth; d += step_size){
			std::cerr << "Output " << output << " Depth " << d << std::endl;
			std::string message = "STREAM_RECONSTRUCTION " + std::to_string(d) + "\n0\n";

			sock->sendMessage(message);

			Sleep(500);

			cv::Mat image = sock->receiveImage();

			std::string name;
			switch (output)
			{
			default:
			case 0:
				name = datafolder + "//Intensity_" + std::to_string(d) + ".ext";
				break;
			case 1:
				name = datafolder + "//Amplitude_" + std::to_string(d) + ".ext";
				break;
			case 2:
				name = datafolder + "//Phase_" + std::to_string(d) + ".ext";
				break;
			}

			FILE* file = fopen(name.c_str(), "wb");
			fwrite(image.data, sizeof(float), image.size().area(), file);
			fclose(file);

			if (show){
				cv::Mat B;
				normalize(image, image, 0, 255, CV_MINMAX);
				image.convertTo(B, CV_8U);
				imshow("Display window", B);
				switch (output)
				{
				default:
				case 0:
					imwrite(datafolder + "//img//Intensity_" + std::to_string(d) + ".png", B);
					break;
				case 1:
					imwrite(datafolder + "//img//Amplitude_" + std::to_string(d) + ".png", B);
					break;
				case 2:
					imwrite(datafolder + "//img//Phase_" + std::to_string(d) + ".png", B);
					break;
				}
				cv::waitKey(1);
			}
		}
	}
}

//extract the max for value and depth
void step2(double _threshold, double _windowsize)
{
	if (show) cv::namedWindow("Max", cv::WINDOW_AUTOSIZE);// Create a window for display.
	if (show) cv::namedWindow("Depth", cv::WINDOW_AUTOSIZE);// Create a window for display.

	std::string name;
	float* maximum = new float[width * height];
	float* depth = new float[width * height];
	cv::Mat image_depth(cv::Size(width, height), CV_32FC1, depth);
	cv::Mat image_maximum(cv::Size(width, height), CV_32FC1, maximum);

	for (int d = min_depth; d <= max_depth; d += step_size){
		name = datafolder + "//Phase_" + std::to_string(d) + ".ext";
			
		float* image = new float[width * height];
		FILE* file = fopen(name.c_str(), "rb");
		fread(image, sizeof(float), width * height, file);
		fclose(file);

		cv::Mat image_in(cv::Size(width, height), CV_32FC1, image);
		Mat kernel = cv::Mat(_windowsize, _windowsize, CV_32FC1, Scalar::all(1.0 / (_windowsize*_windowsize)));
		filter2D(image_in, image_in, CV_32F, kernel);
		if (show)
		{
			cv::Mat image_disp;
			cv::Mat B;
			normalize(image_in, image_disp, 0, 255, CV_MINMAX);
			image_disp.convertTo(B, CV_8U);
			imshow("Filtered", B);
			cv::waitKey(1);
		}
		image_in.release();

		float* max_ptr = maximum;
		float* image_ptr = image;
		float * depth_ptr = depth;

		if (d == min_depth)
		{
			for (int i = 0; i < width * height; i++, max_ptr++, image_ptr++, depth_ptr++)
			{
				if (*image_ptr > _threshold){
					*max_ptr = *image_ptr;
					*depth_ptr = (float)min_depth;
				}
				else
				{
					*max_ptr = 0.0;
					*depth_ptr = 0.0;
				}
			}

		}
		else
		{
			for (int i = 0; i < width * height; i++, max_ptr++, image_ptr++, depth_ptr++)
			{
				if (*max_ptr < *image_ptr && *image_ptr > _threshold){
					*max_ptr = *image_ptr;
					*depth_ptr = (float)d;
				}
			}
		}

		delete[] image;

		if (show)
		{
			cv::Mat image_disp;
			cv::Mat B;
			normalize(image_maximum, image_disp, 0, 255, CV_MINMAX);
			image_disp.convertTo(B, CV_8U);
			imshow("Max", B);
			cv::waitKey(1);

			normalize(image_depth, image_disp, 0, 255, CV_MINMAX);
			image_disp.convertTo(B, CV_8U);
			imshow("Depth", B);
			cv::waitKey(1);
		}
	}

	name = datafolder + "//depth.ext";
	FILE* file = fopen(name.c_str(), "wb");
	fwrite(image_depth.data, sizeof(float), image_depth.size().area(), file);
	fclose(file);

	name = datafolder + "//maximum.ext";
	file = fopen(name.c_str(), "wb");
	fwrite(image_maximum.data, sizeof(float), image_maximum.size().area(), file);
	fclose(file);

	image_depth.release();
	image_maximum.release();

	delete[] maximum;
	delete[] depth;
}

template <typename T> cv::Mat plotGraph(std::vector<T>& vals, int YRange[2])
{
	auto it = std::minmax_element(vals.begin(), vals.end());
	float scale = 1. / ceil(*it.second - *it.first);
	float bias = *it.first;
	int rows = YRange[1] - YRange[0] + 1;
	cv::Mat image = Mat::zeros(rows, vals.size(), CV_8UC3);
	image.setTo(0);
	for (int i = 0; i < (int)vals.size() - 1; i++)
	{
		cv::line(image, cv::Point(i, rows - 1 - (vals[i] - bias)*scale*YRange[1]), cv::Point(i + 1, rows - 1 - (vals[i + 1] - bias)*scale*YRange[1]), Scalar(255, 0, 0), 1);
	}

	return image;
}

//extract the regions of interest
void step3(double _threshold)
{
	std::string name = datafolder + "//maximum.ext";
	float* maximum = new float[width * height];
	FILE* file = fopen(name.c_str(), "rb");
	fread(maximum, sizeof(float), width * height, file);
	fclose(file);

	name = datafolder + "//depth.ext";
	float* depth = new float[width * height];
	file = fopen(name.c_str(), "rb");
	fread(depth, sizeof(float), width * height, file);
	fclose(file);

	cv::Mat image_depth(cv::Size(width, height), CV_32FC1, depth);
	cv::Mat image_maximum(cv::Size(width, height), CV_32FC1, maximum);

	threshold(image_maximum, image_maximum, _threshold, 1., CV_THRESH_BINARY);

	if (show)
	{
		cv::Mat image_disp;
		cv::Mat B;
		normalize(image_maximum, image_disp, 0, 255, CV_MINMAX);
		image_disp.convertTo(B, CV_8U);
		imshow("Filtered", B);
		cv::waitKey(0);
		destroyWindow("Filtered");
	}

	cv::Mat bw;
	image_maximum.convertTo(bw, CV_8U);

	// Perform the distance transform algorithm
	Mat dist;
	distanceTransform(bw, dist, CV_DIST_L2, 5);
	// Normalize the distance image for range = {0.0, 1.0}
	// so we can visualize and threshold it
	normalize(dist, dist, 0, 1., NORM_MINMAX);
	if (show){
		imshow("Distance Transform Image", dist);
		cvWaitKey(0);
		destroyWindow("Distance Transform Image");
	}

	// Threshold to obtain the peaks
	// This will be the markers for the foreground objects
	threshold(dist, dist, 0.05, 1., CV_THRESH_BINARY);
	// Dilate a bit the dist image
	Mat kernel1 = Mat::ones(3, 3, CV_8UC1);
	dilate(dist, dist, kernel1);
	if (show){
		imshow("Peaks", dist);
		cvWaitKey(0);
		destroyWindow("Peaks");
	}

	// Create the CV_8U version of the distance image
	// It is needed for findContours()
	Mat dist_8u;
	dist.convertTo(dist_8u, CV_8U);
	// Find total markers
	std::vector<std::vector<Point> > contours_org;
	std::vector<std::vector<Point> > contours;
	std::vector<cv::Rect> bounds;
	findContours(dist_8u, contours_org, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	//filter by Area and position
	for (size_t i = 0; i < contours_org.size(); i++)
	{
		cv::Moments mu = moments(contours_org[i], false);
		cv::Point2f center = cv::Point2f(mu.m10 / mu.m00, mu.m01 / mu.m00);
		if (cv::contourArea(contours_org[i]) >= contour_minArea 
			&& center.x > border_minDist && center.x < width-border_minDist 
			&& center.y > border_minDist && center.y < height - border_minDist)
		{
			contours.push_back(contours_org[i]);
			cv::Rect boundingBox = boundingRect(contours_org[i]);
			bounds.push_back(boundingBox);
		}
	}
	std::cerr << " Contours found " << contours.size() << std::endl;

	if (show){
		Mat drawing = Mat::zeros(dist_8u.size(), CV_8UC3);
		for (int i = 0; i< contours.size(); i++)
		{
			Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
			drawContours(drawing, contours, i, color, 2, 8, 0, 0, Point());
			rectangle(drawing, bounds[i].tl(), bounds[i].br(), color, 2, 8, 0);
		}
		imshow("Contours", drawing);
		cvWaitKey(0);
		destroyWindow("Contours");
	}

	//find best depth
	for (size_t i = 0; i < contours.size(); i++)
	{
		std::vector<float> depth_values;
		int depth_cont = 0;
		float maxVal = 0;
		for (int d = min_depth; d <= max_depth; d += step_size){
			name = datafolder + "//Phase_" + std::to_string(d) + ".ext";

			float* image = new float[width * height];
			FILE* file = fopen(name.c_str(), "rb");
			fread(image, sizeof(float), width * height, file);
			fclose(file);
			cv::Mat image_in(cv::Size(width, height), CV_32FC1, image);

			cv::Mat roi = image_in(bounds[i]);
			float val = sum(roi)[0] / bounds[i].area();
			depth_values.push_back(val);
			std::cerr << val << std::endl;
			if (val > maxVal)
			{
				maxVal = val;
				depth_cont = d;

				/*if (show)
				{
					cv::Mat image_disp;
					cv::Mat B;
					normalize(roi, image_disp, 0, 255, CV_MINMAX);
					image_disp.convertTo(B, CV_8U);
					imshow("Max", B);
					cv::waitKey(1);
				}*/
			}

			image_in.release();
			delete[] image;
		}

		name =  "d:\\data//Amplitude_" + std::to_string(depth_cont) + ".ext";

		float* image = new float[width * height];
		FILE* file = fopen(name.c_str(), "rb");
		fread(image, sizeof(float), width * height, file);
		fclose(file);
		cv::Mat image_in(cv::Size(width, height), CV_32FC1, image);
		cv::Rect bound_cont = bounds[i];
		bound_cont.x = bound_cont.x - bound_cont.width;
		bound_cont.y = bound_cont.y - bound_cont.height;
		bound_cont.width = 3 * bound_cont.width;
		bound_cont.height = 3 * bound_cont.height;

		cv::Mat roi = image_in(bound_cont);

		if (show)
		{
			int range[2] = { 0, 100 };
			cv::Mat lineGraph = plotGraph(depth_values, range);
			imshow("vals", lineGraph);
			cv::waitKey(0);
			cv::Mat image_disp;
			cv::Mat B;
			normalize(roi, image_disp, 0, 255, CV_MINMAX);
			image_disp.convertTo(B, CV_8U);
			imwrite(datafolder + "//out_" + std::to_string(i) + "_"+ std::to_string(depth_cont) + ".png", B);
		}
	}

	image_depth.release();
	image_maximum.release();

	delete[] maximum;
	delete[] depth;
}

int main(int argc, char** argv)
{
	for (int step = step_start; step <= step_end; step++){
		//save Data from Octopus
		if (step == 0){
			step1();
		}
		//find Maximum phase
		else if (step == 1)
		{
			step2(0, 5);
		}
		//Extract Markers
		else if (step == 2){
			step3(1);	
		}
			

			
			//RNG rng(12345);

			//cv::Mat depth_normed;
			//cv::Mat bw;
			//normalize(image_depth, depth_normed, 0, 255, CV_MINMAX);
			//depth_normed.convertTo(bw, CV_8U);
			//Mat src;
			////cvtColor(bw, src, CV_GRAY2RGB);

			//threshold(bw, bw, 0.1, 1., CV_THRESH_BINARY);
			//// Perform the distance transform algorithm
			//Mat dist;
			//distanceTransform(bw, dist, CV_DIST_L2, 5);
			//// Normalize the distance image for range = {0.0, 1.0}
			//// so we can visualize and threshold it
			//normalize(dist, dist, 0, 1., NORM_MINMAX);
			//if (show){
			//	imshow("Distance Transform Image", dist);
			//	cvWaitKey(0);
			//	destroyWindow("Distance Transform Image");
			//}

			//// Threshold to obtain the peaks
			//// This will be the markers for the foreground objects
			//threshold(dist, dist, 0.1, 1., CV_THRESH_BINARY);
			//// Dilate a bit the dist image
			//Mat kernel1 = Mat::ones(3, 3, CV_8UC1);
			//dilate(dist, dist, kernel1);
			//if (show){
			//	imshow("Peaks", dist);
			//	cvWaitKey(0);
			//	destroyWindow("Peaks");
			//}

			//// Create the CV_8U version of the distance image
			//// It is needed for findContours()
			//Mat dist_8u;
			//dist.convertTo(dist_8u, CV_8U);
			//// Find total markers
			//std::vector<std::vector<Point> > contours_org;
			//std::vector<std::vector<Point> > contours;
			//findContours(dist_8u, contours_org, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

			////filter by Area and position
			//for (size_t i = 0; i < contours_org.size(); i++)
			//{
			//	cv::Moments mu = moments(contours_org[i], false);
			//	cv::Point2f center = cv::Point2f(mu.m10 / mu.m00, mu.m01 / mu.m00);
			//	if (cv::contourArea(contours_org[i]) >= contour_minArea 
			//		&& center.x > border_minDist && center.x < width-border_minDist 
			//		&& center.y > border_minDist && center.y < height - border_minDist)
			//	{
			//		contours.push_back(contours_org[i]);
			//	}
			//}
			//std::cerr << " Contours found " << contours.size() << std::endl;
			//std::vector <float> mean_depths;

			////get meanDepth per contour
			//for (size_t i = 0; i < contours.size(); i++)
			//{
			//	float depth_val = 0;
			//	int count = 0;

			//	Mat tmp_image = Mat::zeros(dist.size(), CV_8UC1);
			//	drawContours(tmp_image, contours, static_cast<int>(i), Scalar::all(static_cast<int>(255)), -1);
			//	cv::Rect boundRect = boundingRect(contours[i]);
			//	
			//	for (int y = boundRect.tl().y; y <= boundRect.br().y ; y++)
			//	{
			//		for (int x = boundRect.tl().x; x <= boundRect.br().x; x++)
			//		{
			//			if (tmp_image.at<uchar>(y, x)>0){
			//				depth_val += image_depth.at<float>(y, x);
			//				count++;
			//			}
			//		}
			//	}
			//	mean_depths.push_back(depth_val / count);
			//}
			//
			////get image per contour
			//for (size_t i = 0; i < contours.size(); i++)
			//{
			//	float depth_val = mean_depths[i];
			//	depth_val = round(depth_val / step_size)*step_size;
			//	std::cerr << "closest depth " << depth_val << std::endl;
			//	name = datafolder + "//Phase_" + std::to_string((int)depth_val) + ".ext";
			//	float* amp = new float[width * height];
			//	file = fopen(name.c_str(), "rb");
			//	fread(amp, sizeof(float), width * height, file);
			//	fclose(file);

			//	cv::Mat image_amp(cv::Size(width, height), CV_32FC1, amp);

			//	// Create the marker image for the watershed algorithm
			//	Mat markers = Mat::zeros(image_amp.size(), CV_32SC1);
			//	// Draw the foreground markers
			//	drawContours(markers, contours, static_cast<int>(i), Scalar::all(static_cast<int>(1)), -1);

			//	
			//	// Draw the background marker
			//	cv::Rect boundRect = boundingRect(contours[i]);

			//	for (int y = 0; y < markers.rows; y++)
			//	{
			//		for (int x = 0; x < markers.cols; x++)
			//		{
			//			if (!(x > boundRect.tl().x - 20 && x < boundRect.br().x + 20 &&
			//				y > boundRect.tl().y - 20 && y < boundRect.br().y + 20)){
			//				markers.at<int>(y, x) = 255;
			//			}
			//		}
			//	}


			//	Mat src;
			//	Mat src2;
			//	cv::Mat image_disp;
			//	normalize(image_amp, image_disp, 0, 255, CV_MINMAX);
			//	image_disp.convertTo(src2, CV_8UC3);
			//	
			//	cvtColor(src2, src, CV_GRAY2RGB);

			//	boundRect.x = boundRect.x - 20;
			//	boundRect.y = boundRect.y - 20;
			//	boundRect.width = boundRect.width + 40;
			//	boundRect.height = boundRect.height + 40;

			//	imwrite(datafolder + "//out_" + std::to_string(i) + ".png", src(boundRect));
			//	watershed(src, markers);

			//	for (int i = 0; i < markers.rows; i++)
			//		{
			//			for (int j = 0; j < markers.cols; j++)
			//			{
			//				int index = markers.at<int>(i, j);
			//				if (index == 1){
			//					src.at<Vec3b>(i, j) = Vec3b(255,0,0);
			//				}
			//			}
			//		}

			//	if (show)
			//	{
			//		imshow("Image", src);
			//		cv::waitKey(0);
			//	}

			//	image_amp.release();
			//	delete[] amp;
			//}

			

			//// Perform the watershed algorithm
			//watershed(src, markers);

			//// Generate random colors
			//std::vector<Vec3b> colors;
			//for (size_t i = 0; i < contours.size(); i++)
			//{
			//	int b = theRNG().uniform(0, 255);
			//	int g = theRNG().uniform(0, 255);
			//	int r = theRNG().uniform(0, 255);
			//	colors.push_back(Vec3b((uchar)b, (uchar)g, (uchar)r));
			//}

			//// Create the result image
			//Mat dst = Mat::zeros(markers.size(), CV_8UC3);
			//// Fill labeled objects with random colors
			//int maxIndex = 0;
			//for (int i = 0; i < markers.rows; i++)
			//{
			//	for (int j = 0; j < markers.cols; j++)
			//	{
			//		int index = markers.at<int>(i, j);
			//		if (index > maxIndex) maxIndex = index;
			//		if (index == 0){
			//			dst.at<Vec3b>(i, j) = colors[index];
			//		}
			//		/*if (index >= 1 && index <= static_cast<int>(contours.size())){
			//			dst.at<Vec3b>(i, j) = colors[index - 1];
			//		}
			//		else{
			//			dst.at<Vec3b>(i, j) = Vec3b(0, 0, 0);
			//		}*/
			//	}
			//}
			//std::cerr << "MaxIndex " << maxIndex << "Contour size " << contours.size() << std::endl;
			//
			////for (int i = 0; i< contours.size(); i++)
			////{
			////	Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
			////	drawContours(dst, contours, i, color, 2, 8, 0, 0, Point());
			////}

			//// Visualize the final image
			//if (show){
			//	imshow("Final Result", dst);
			//	cvWaitKey(0);
			//	destroyWindow("Final Result");
			//}

			//// determine mean depth per marker
			//std::vector<std::vector<float> > depth_markers;
			//for (int i = 0; i < contours.size(); i++)
			//{
			//	std::vector<float> d_marker;
			//	depth_markers.push_back(d_marker);
			//}

			//for (int i = 0; i < markers.rows; i++)
			//{
			//	for (int j = 0; j < markers.cols; j++)
			//	{
			//		int index = markers.at<int>(i, j);
			//		 
			//		if (index >= 1 && index <= static_cast<int>(contours.size())){
			//			depth_markers[index -  1].push_back(image_depth.at<float>(i, j));
			//		}
			//	}
			//}

			//std::vector<float> mean_depth_markers;
			//for (int i = 0; i < depth_markers.size(); i++)
			//{
			//	int mean = 0;
			//	for (int j = 0; j < depth_markers[i].size(); j++)
			//	{
			//		mean += depth_markers[i][j];
			//	}

			//	mean_depth_markers.push_back(mean / depth_markers[i].size());
			//}

			
	}
	return 0;
}

