///\file OfflineReader.h
///\author Benjamin Knorlein
///\date 11/13/2016

#ifndef OFFLINEREADER_H
#define OFFLINEREADER_H

#include "ImageSource.h"

class OfflineReader : public ImageSource {
public:
	virtual bool setSourceHologram(std::string folder, std::string filename, std::string background);
	virtual void getPhaseImage(int depth, float* data);
	virtual void getIntensityImage(int depth, float* data);
	virtual void getAmplitudeImage(int depth, float* data);
};

#endif //OFFLINEREADER_H