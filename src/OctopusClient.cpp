///\file OctopusClient.cpp
///\author Benjamin Knorlein
///\date 11/13/2016

#include "OctopusClient.h"

#include <iostream>
#include <assert.h>

OctopusClient::OctopusClient(const std::string &serverIP, const std::string &serverPort) : m_mode(-1)
{
	m_sock = new Socket(serverIP, serverPort);


	// Send the API version to the Octopus software.  This is supposed to 
	// put the software into the special 'receive information through
	// back door' mode, and should more or less disable the GUI.

	m_sock->sendMessage("SET_API_VERSION 2\n");

	//clear pipe;
	char buffer[1024];

	int n;
	while (((n = recv(*m_sock->getSocketFD(), buffer, sizeof(buffer), 0))>0))
	{
		if (n <= 0)
		{
			break;
		}
	}
}

OctopusClient::~OctopusClient()
{
	delete m_sock;
}

void OctopusClient::getPhaseImage(int depth, float* data)
{
	setOutputMode(2);
	receiveImageData(depth, data);
}
void OctopusClient::getIntensityImage(int depth, float* data)
{
	setOutputMode(0);
	receiveImageData(depth, data);
}
void OctopusClient::getAmplitudeImage(int depth, float* data)
{
	setOutputMode(1);
	receiveImageData(depth, data);
}

bool OctopusClient::setSourceHologram(std::string folder, std::string filename, std::string background)
{
	m_background = background;
	m_filename = filename;
	m_folder = folder;
	std::string backgroundString = "";
	if (m_background != "")
	{
		backgroundString = "*" + m_folder + m_background;
	}
	m_sock->sendMessage("RECONSTRUCT_HOLOGRAMS " + m_folder + m_filename + backgroundString + "\n0\n");

	Sleep(10);
	m_sock->receiveMessage(&m_dummyBuffer[0], 26);

	std::string reply = std::string(m_dummyBuffer, 26);
	if (reply != "RECONSTRUCT_HOLOGRAMS 0\n0\n" && reply != "RECONSTRUCT_HOLOGRAMS 1\n0\n" && reply != "RECONSTRUCT_HOLOGRAMS 2\n0\n") {
		std::cout << "REPLY:" << reply << ", expect:" << "RECONSTRUCT_HOLOGRAMS 0\n0\n or RECONSTRUCT_HOLOGRAMS 1\n0\n or RECONSTRUCT_HOLOGRAMS 2\n0\n" << std::endl;
		assert(reply == "RECONSTRUCT_HOLOGRAMS 0\n0\n" || reply == "RECONSTRUCT_HOLOGRAMS 1\n0\n" || reply == "RECONSTRUCT_HOLOGRAMS 2\n0\n");
	}

	if (reply != "RECONSTRUCT_HOLOGRAMS 1\n0\n") return false;
#ifdef DEBUG
	std::cout << reply << std::endl;
#endif
	Sleep(500);
	return true;
}

void OctopusClient::receiveImageData(int depth, float* data)
{
	int datasize = 4 * 2048 * 2048;

	std::string message = "STREAM_RECONSTRUCTION " + std::to_string(depth) + "\n0\n";
#ifdef DEBUG
	std::cout << "Send" << message << std::endl;
#endif
	m_sock->sendMessage(message);

	//receive data
	std::string expectedReply = "STREAM_RECONSTRUCTION 2048 2048 " + m_filename + " " + m_background + " " + std::to_string(depth) + " " + std::to_string(m_mode) + "\n" + std::to_string(datasize) + "\n";

	Sleep(500);
	m_sock->receiveMessage(m_dummyBuffer, expectedReply.size());
#ifdef DEBUG
	std::cout << "dummyBuffer:" << m_dummyBuffer << std::endl;
#endif
	std::string reply = std::string(m_dummyBuffer, expectedReply.size());
	if (reply != expectedReply) {
		std::cout << "REPLY:" << reply << ":vs XPCT:" << expectedReply << std::endl;
		assert(reply == expectedReply);
	}
#ifdef DEBUG
	std::cout << "Receive " << reply << std::endl;
#endif
	char * dataPtr = (char *)&data[0];
	datasize = m_sock->receiveMessage(dataPtr, datasize);

	//int * ptr_org = (int*)&data[0];
	//int * ptr_dest = (int*)&data[0];

	//for (int i = 0; i < datasize / 4; i++, ptr_org++, ptr_dest++)
	//{
	//	*ptr_dest = (*ptr_org << 24) |
	//		((*ptr_org << 8) & 0x00ff0000) |
	//		((*ptr_org >> 8) & 0x0000ff00) |
	//		((*ptr_org >> 24) & 0x000000ff);
	//}
}

bool OctopusClient::setOutputMode(int mode)
{
	if (m_mode != mode){
		m_mode = mode;
		std::string message = "OUTPUT_MODE " + std::to_string(m_mode) + "\n0\n";
		m_sock->sendMessage(message);

		m_sock->receiveMessage(&m_dummyBuffer[0], message.size());

		std::string reply = std::string(m_dummyBuffer, message.size());
#ifdef DEBUG
		std::cout << reply << std::endl;
#endif
		assert(reply == message);
		if (reply != message) return false;
		Sleep(500);
	}

	return true;
}