///\file OctopusClient.h
///\author Benjamin Knorlein
///\date 11/13/2016

#ifndef OCTOPUSCLIENT_H
#define OCTOPUSCLIENT_H

#include "ImageSource.h"
#include "Socket.h"

class OctopusClient : public ImageSource{
public:
	OctopusClient(const std::string &serverIP, const std::string &serverPort);
	virtual	~OctopusClient();

	virtual bool setSourceHologram(std::string folder, std::string filename, std::string background = "");
	virtual void getPhaseImage(int depth, float* data);
	virtual void getIntensityImage(int depth, float* data);
	virtual void getAmplitudeImage(int depth, float* data);

private:
	void receiveImageData(int depth, float* data);
	bool setOutputMode(int mode);

	int m_mode;
	Socket * m_sock;

	char m_dummyBuffer[1024];

	bool m_withoutBackground;
};

#endif //OCTOPUSCLIENT_H
