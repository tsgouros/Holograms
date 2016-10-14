///\file ReportWriter.cpp
///\author Benjamin Knorlein
///\date 11/13/2016

#include "ReportWriter.h"
#include <iostream>
#include <fstream>
#include <opencv2/imgcodecs.hpp>

ReportWriter::ReportWriter(std::string outdir, std::string filename, std::string templateFolder) : m_outdir(outdir), m_filename(filename), m_templateFolder(templateFolder)
{
	

}

ReportWriter::~ReportWriter()
{
	
}

void ReportWriter::writeXMLReport(std::vector<cv::Rect> bounds, std::vector<int> depths_contour, std::vector<double> vals_contour, double time)
{
	std::ifstream infile;
	std::string templateFile = "template.xml";
	if (!m_templateFolder.empty()) templateFile = m_templateFolder + "/" + templateFile;

	infile.open(templateFile, std::ifstream::in);

	char * buffer;
	long size;

	// get size of file
	infile.seekg(0);
	std::streampos fsize = 0;
	infile.seekg(0, infile.end);
	size = infile.tellg() - fsize;
	infile.seekg(0);

	// allocate memory for file content
	buffer = new char[size];

	// read content of infile
	infile.read(buffer, size);

	infile.close();

	std::ofstream outfile(m_outdir + "/" + "report.xml", std::ofstream::out);
	// write to outfile
	std::string buffer_st(buffer);
	outfile.write(buffer, size);
	delete[] buffer;

	outfile << "<DATA>" << std::endl;
	outfile << "<FILENAME>" << m_filename << "</FILENAME>" << std::endl;
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
		outfile << "<IMAGE>" << "contours_" + std::to_string(((long long)c)) + ".png" << "</IMAGE>" << std::endl;
		outfile << "<IMAGEPHASE>" << "contoursPhase_" + std::to_string(((long long)c)) + ".png" << "</IMAGEPHASE>" << std::endl;
		outfile << "</ROI>" << std::endl;
	}
	outfile << "</DATA>" << std::endl;
	outfile << "</doc>" << std::endl;

	outfile.close();
}

void ReportWriter::saveROIImages(ImageCache* cache, std::vector<cv::Rect> bounds, std::vector<int> depths_contour)
{
	for (size_t c = 0; c < bounds.size(); c++)
	{
		std::cout << "Save Contour " << c << " at depth " << std::to_string(((long long)depths_contour[c])) << std::endl;
		int d = depths_contour[c];
		cv::Mat *image = cache->getPhaseImage(d);

		cv::Rect bound_cont = bounds[c];
		bound_cont.x = bound_cont.x - 20;
		bound_cont.y = bound_cont.y - 20;
		bound_cont.width = bound_cont.width + 40;
		bound_cont.height = bound_cont.height + 40;

		if (bound_cont.x < 0) bound_cont.x = 0;
		if (bound_cont.y < 0) bound_cont.y = 0;
		if (bound_cont.y + bound_cont.height > image->rows) bound_cont.height = image->rows - bound_cont.y;
		if (bound_cont.x + bound_cont.width > image->cols) bound_cont.width = image->cols - bound_cont.x;

		cv::Mat roi = (*image)(bound_cont);

		cv::Mat image_display;
		cv::Mat drawing;
		normalize(roi, image_display, 0, 255, CV_MINMAX);
		image_display.convertTo(drawing, CV_8U);

		cv::imwrite(m_outdir + "/" + "contoursPhase_" + std::to_string(((long long)c)) + ".png", drawing);
	}

	for (size_t c = 0; c < bounds.size(); c++)
	{
		std::cout << "Save Contour " << c << " at depth " << std::to_string(((long long)depths_contour[c])) << std::endl;
		int d = depths_contour[c];
		cv::Mat *image = cache->getAmplitudeImage(d);

		cv::Rect bound_cont = bounds[c];
		bound_cont.x = bound_cont.x - 20;
		bound_cont.y = bound_cont.y - 20;
		bound_cont.width = bound_cont.width + 40;
		bound_cont.height = bound_cont.height + 40;

		if (bound_cont.x < 0) bound_cont.x = 0;
		if (bound_cont.y < 0) bound_cont.y = 0;
		if (bound_cont.y + bound_cont.height > image->rows) bound_cont.height = image->rows - bound_cont.y;
		if (bound_cont.x + bound_cont.width > image->cols) bound_cont.width = image->cols - bound_cont.x;

		cv::Mat roi = (*image)(bound_cont);

		cv::Mat image_display;
		cv::Mat drawing;
		normalize(roi, image_display, 0, 255, CV_MINMAX);
		image_display.convertTo(drawing, CV_8U);

		cv::imwrite(m_outdir + "/" + "contours_" + std::to_string(((long long)c)) + ".png", drawing);
	}
}

void ReportWriter::saveImage(cv::Mat image, std::string filename, bool normalizeImage)
{
	if (normalizeImage){
		cv::Mat image_disp;
		cv::Mat B;
		normalize(image, image_disp, 0, 255, CV_MINMAX);
		image_disp.convertTo(B, CV_8U);

		imwrite(m_outdir + "/" + filename, B);
	} 
	else
	{
		imwrite(m_outdir + "/" + filename, image);
	}
}
