///\file ImageSource.h
///\author Benjamin Knorlein
///\date 11/13/2016

#ifndef IMAGESOURCE_H
#define IMAGESOURCE_H

#include <string>

class ImageSource {
	public:
		virtual bool setSourceHologram(std::string folder, std::string filename, std::string background = "") = 0;
		virtual void getPhaseImage(int depth, float* data)  = 0;
		virtual void getIntensityImage(int depth, float* data) = 0;
		virtual void getAmplitudeImage(int depth, float* data) = 0;

		std::string getFolder(){ return m_folder; }
		std::string getFilename(){ return m_filename; }

	protected:
		std::string m_folder;
		std::string m_filename;
		std::string m_background;
};

#endif //IMAGESOURCE_H