///\file Settings.cpp
///\author Benjamin Knorlein
///\date 11/14/2016

#include "Settings.h"
#include "tinyxml2.h"
#include <iostream>

Settings::Settings(const char* filename)
{
	std::string templateFolder = "";

	//octopus
	online = false;
	
	//ui
	show = true;

	//refinement
	doRefine = false;

	//general
	useAbs = true;
	step_size = 100;
	min_depth = 1000;
	max_depth = 25000;
	width = 2048;
	height = 2048;

	//max image
	windowsize = 5;

	//contours
	max_threshold = 0.35;
	contour_minArea = 10.0;

	//merge
	doMergebounds = true;
	merge_threshold_depth = 400;
	merge_threshold_dist = 50;

	tinyxml2::XMLDocument doc;
	doc.LoadFile(filename);

	tinyxml2::XMLElement* titleElement;
	
	titleElement = doc.FirstChildElement("Settings")->FirstChildElement("datafolder");
	if (titleElement) {
		datafolder = (titleElement->GetText() != 0) ? titleElement->GetText() : "";
		std::cerr << "datafolder = " << datafolder << std::endl;
	}

	titleElement = doc.FirstChildElement("Settings")->FirstChildElement("outputFolder");
	if (titleElement) {
		outputFolder = (titleElement->GetText() != 0) ? titleElement->GetText() : "";
		std::cerr << "outputFolder = " << outputFolder  << std::endl;
	}

	titleElement = doc.FirstChildElement("Settings")->FirstChildElement("templateFolder");
	if (titleElement) {
		templateFolder = (titleElement->GetText() != 0) ? titleElement->GetText() : "";
		std::cerr << "templateFolder = " << templateFolder << std::endl;
	}

	titleElement = doc.FirstChildElement("Settings")->FirstChildElement("online");
	if (titleElement) {
		online = std::stoi(std::string(titleElement->GetText()));
		std::cerr << "online = " << online << std::endl;
	}

	titleElement = doc.FirstChildElement("Settings")->FirstChildElement("ip");
	if (titleElement) {
		ip = (titleElement->GetText() != 0) ? titleElement->GetText() : "";
		std::cerr << "ip = " << ip << std::endl;
	}

	titleElement = doc.FirstChildElement("Settings")->FirstChildElement("port");
	if (titleElement) {
		port = (titleElement->GetText() != 0) ? titleElement->GetText() : "";
		std::cerr << "port = " << port << std::endl;
	}

	titleElement = doc.FirstChildElement("Settings")->FirstChildElement("show");
	if (titleElement) {
		show = std::stoi(std::string(titleElement->GetText()));
		std::cerr << "show = " << show << std::endl;
	}

	titleElement = doc.FirstChildElement("Settings")->FirstChildElement("useAbs");
	if (titleElement) {
		useAbs = std::stoi(std::string(titleElement->GetText()));
		std::cerr << "useAbs = " << useAbs << std::endl;
	}

	titleElement = doc.FirstChildElement("Settings")->FirstChildElement("step_size");
	if (titleElement) {
		step_size = std::stoi(std::string(titleElement->GetText()));
		std::cerr << "step_size = " << step_size << std::endl;
	}

	titleElement = doc.FirstChildElement("Settings")->FirstChildElement("min_depth");
	if (titleElement) {
		min_depth = std::stoi(std::string(titleElement->GetText()));
		std::cerr << "min_depth = " << min_depth << std::endl;
	}

	titleElement = doc.FirstChildElement("Settings")->FirstChildElement("max_depth");
	if (titleElement) {
		max_depth = std::stoi(std::string(titleElement->GetText()));
		std::cerr << "max_depth = " << max_depth << std::endl;
	}

	titleElement = doc.FirstChildElement("Settings")->FirstChildElement("width");
	if (titleElement) {
		width = std::stoi(std::string(titleElement->GetText()));
		std::cerr << "width = " << width << std::endl;
	}

	titleElement = doc.FirstChildElement("Settings")->FirstChildElement("height");
	if (titleElement) {
		height = std::stoi(std::string(titleElement->GetText()));
		std::cerr << "height = " << height << std::endl;
	}

	titleElement = doc.FirstChildElement("Settings")->FirstChildElement("doRefine");
	if (titleElement) {
		doRefine = std::stoi(std::string(titleElement->GetText()));
		std::cerr << "doRefine = " << doRefine << std::endl;
	}

	titleElement = doc.FirstChildElement("Settings")->FirstChildElement("windowsize");
	if (titleElement) {
		windowsize = std::stoi(std::string(titleElement->GetText()));
		std::cerr << "windowsize = " << windowsize << std::endl;
	}

	titleElement = doc.FirstChildElement("Settings")->FirstChildElement("max_threshold");
	if (titleElement) {
		max_threshold = std::stof(std::string(titleElement->GetText()));
		std::cerr << "max_threshold = " << max_threshold << std::endl;
	}

	titleElement = doc.FirstChildElement("Settings")->FirstChildElement("contour_minArea");
	if (titleElement) {
		contour_minArea = std::stof(std::string(titleElement->GetText()));
		std::cerr << "contour_minArea = " << contour_minArea << std::endl;
	}

	titleElement = doc.FirstChildElement("Settings")->FirstChildElement("doMergebounds");
	if (titleElement) {
		doMergebounds = std::stoi(std::string(titleElement->GetText()));
		std::cerr << "doMergebounds = " << doMergebounds << std::endl;
	}

	titleElement = doc.FirstChildElement("Settings")->FirstChildElement("merge_threshold_depth");
	if (titleElement) {
		merge_threshold_depth = std::stoi(std::string(titleElement->GetText()));
		std::cerr << "merge_threshold_depth = " << merge_threshold_depth << std::endl;
	}

	titleElement = doc.FirstChildElement("Settings")->FirstChildElement("merge_threshold_dist");
	if (titleElement) {
		merge_threshold_dist = std::stoi(std::string(titleElement->GetText()));
		std::cerr << "merge_threshold_dist = " << merge_threshold_dist << std::endl;
	}
}

Settings::~Settings()
{

}