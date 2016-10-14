///\file Settings.h
///\author Benjamin Knorlein
///\date 11/14/2016

#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>

class Settings {
public:

	Settings(const char* filename);
	~Settings();

	const std::string& getDatafolder() const
	{
		return datafolder;
	}

	const std::string& getOutputFolder() const
	{
		return outputFolder;
	}

	const std::string& getTemplateFolder() const
	{
		return templateFolder;
	}

	const bool& getOnline() const
	{
		return online;
	}

	const std::string& getIp() const
	{
		return ip;
	}

	const std::string& getPort() const
	{
		return port;
	}

	const bool& getShow() const
	{
		return show;
	}

	const bool& getUseAbs() const
	{
		return useAbs;
	}

	const int& getStepSize() const
	{
		return step_size;
	}

	const int& getMinDepth() const
	{
		return min_depth;
	}

	const int& getMaxDepth() const
	{
		return max_depth;
	}

	const int& getWidth() const
	{
		return width;
	}

	const int& getHeight() const
	{
		return height;
	}

	const bool& getDoRefine() const
	{
		return doRefine;
	}

	const int& getWindowsize() const
	{
		return windowsize;
	}

	const float& getMaxThreshold() const
	{
		return max_threshold;
	}

	const double& getContourMinArea() const
	{
		return contour_minArea;
	}

	const bool& getDoMergebounds() const
	{
		return doMergebounds;
	}

	const float& getMergeThresholdDepth() const
	{
		return merge_threshold_depth;
	}

	const float& getMergeThresholdDist() const
	{
		return merge_threshold_dist;
	}

protected:

protected:

	//Input Output
	std::string datafolder;

	std::string outputFolder;
	std::string templateFolder;

	//Octopus
	bool online;
	std::string ip;
	std::string port;
	
	//ui
	bool show;
	
	//general processing
	bool useAbs;
	int step_size;
	int min_depth;
	int max_depth;
	int width;
	int height;

	//refinement
	bool doRefine;

	//max image
	int windowsize;

	//contours
	float max_threshold;
	double contour_minArea;

	//merge
	bool doMergebounds;
	float merge_threshold_depth;
	float merge_threshold_dist;
};

#endif //SETTINGS_H