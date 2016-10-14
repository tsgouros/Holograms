///\file ReportWriter.h
///\author Benjamin Knorlein
///\date 11/13/2016

#ifndef REPORTWRITER_H
#define REPORTWRITER_H

#include <string>
#include "ImageCache.h"

#include <opencv2/core/core.hpp>

class ReportWriter {
public:
	ReportWriter(std::string outdir, std::string filename, std::string templateFolder = "");
	~ReportWriter();

	void writeXMLReport(std::vector<cv::Rect> bounds, std::vector<int> depths_contour, std::vector<double> vals_contour, double time);
	void saveROIImages(ImageCache* cache, std::vector<cv::Rect> bounds, std::vector<int> depths_contour);
	void saveImage(cv::Mat image, std::string filename, bool normalizeImage = false);

private:
	std::string m_outdir;
	std::string m_filename;
	std::string m_templateFolder;
};

#endif //REPORTWRITER_H