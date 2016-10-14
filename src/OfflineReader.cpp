///\file OfflineReader.cpp
///\author Benjamin Knorlein
///\date 11/13/2016

#include "OfflineReader.h"
#include <iostream>
#include <iostream>

bool OfflineReader::setSourceHologram(std::string folder, std::string filename, std::string background)
{
	m_background = background;
	m_folder = folder;
	m_filename = filename;
	return true;
}

void OfflineReader::getPhaseImage(int depth, float* data)
{
	std::string name = m_folder + "/" + m_filename + "/" + "Phase_" + std::to_string(depth) + ".ext";

	std::cerr << "Load " << name << std::endl;
	FILE* file = fopen(name.c_str(), "rb");
	
	// obtain file size:
	fseek(file, 0, SEEK_END);
	long lSize = ftell(file);
	rewind(file);

	
	fread(data, sizeof(float), lSize / 4, file);
	fclose(file);
}

void OfflineReader::getIntensityImage(int depth, float* data)
{
	std::string name = m_folder + "/" + m_filename + "/" + "Intensity_" + std::to_string(depth) + ".ext";

	std::cerr << "Load " << name << std::endl;

	FILE* file = fopen(name.c_str(), "rb");

	// obtain file size:
	fseek(file, 0, SEEK_END);
	long lSize = ftell(file);
	rewind(file);


	fread(data, sizeof(float), lSize / 4, file);
	fclose(file);
}

void OfflineReader::getAmplitudeImage(int depth, float* data)
{
	std::string name = m_folder + "/" + m_filename + "/" + "Amplitude_" + std::to_string(depth) + ".ext";
	
	std::cerr << "Load " << name << std::endl;

	FILE* file = fopen(name.c_str(), "rb");

	// obtain file size:
	fseek(file, 0, SEEK_END);
	long lSize = ftell(file);
	rewind(file);


	fread(data, sizeof(float), lSize / 4, file);
	fclose(file);
}
