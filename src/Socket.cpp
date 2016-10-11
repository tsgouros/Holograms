#include "Socket.h"
#include <iostream>


#ifdef _MSC_VER
#include <windows.h>
#endif

#define MAX_FLUSH_CYCLE 10
#define OCTOTERM "\n0\n"

bool DEBUG = false;

#ifndef WIN32

// Provides a socket address in the appropriate (IPv4 or IPv6) format.
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

#endif

// Create the socket, connected to the given IP and port.
Socket::Socket(const std::string& serverIP, const std::string& serverPort)
{
	printf("client: connecting...\n");
#ifdef WIN32  // WinSock implementation

	WSADATA wsaData;
	SOCKET sockfd = INVALID_SOCKET;
	struct addrinfo hints, *servinfo, *p;
	int rv;

	rv = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (rv != 0) {
		std::cerr << "WSAStartup failed with error: " << rv << std::endl;
		exit(1);
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if ((rv = getaddrinfo(serverIP.c_str(), serverPort.c_str(), &hints, &servinfo)) != 0) {
		std::cerr << "getaddrinfo() failed with error: " << rv << std::endl;
		WSACleanup();
		exit(1);
	}

	// loop through all the results and connect to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == INVALID_SOCKET) {
			std::cerr << "socket() failed with error: " << WSAGetLastError() << std::endl;
			continue;
		}

		if (connect(sockfd, p->ai_addr, (int)p->ai_addrlen) == SOCKET_ERROR) {
			closesocket(sockfd);
			sockfd = INVALID_SOCKET;
			std::cerr << "connect() to server socket failed" << std::endl;
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		//return 2;
		exit(2);
	}

	printf("client: connected\n");
	int iTimeout = 3000;
	int out = setsockopt(sockfd,
		SOL_SOCKET,
		SO_RCVTIMEO,
		/*
		reinterpret_cast<char*>(&tv),
		sizeof(timeval) );
		*/
		(const char *)&iTimeout,
		sizeof(iTimeout));

	freeaddrinfo(servinfo); // all done with this structure

	_socketFD = sockfd;


#else  // BSD sockets implementation


	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(serverIP.c_str(), serverPort.c_str(), 
			      &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		//return 1;
		exit(1);
	}

	// loop through all the results and connect to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, 
				     p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		//return 2;
		exit(2);
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), 
		  s, sizeof s);
	printf("client: connected to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	_socketFD = sockfd;

	// Set a timeout.  This was tuned by hand, no idea if optimal.
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 300000;
	setsockopt(_socketFD, SOL_SOCKET, SO_RCVTIMEO, 
		   (char *)&tv, sizeof(struct timeval)); 

#endif


	// Send the API version to the Octopus software.  This is supposed to 
	// put the software into the special 'receive information through
	// back door' mode, and should more or less disable the GUI.

	sendMessage("SET_API_VERSION 2\n");

	//clear pipe;
	receiveMessage();
}

Socket::~Socket()
{
#ifdef WIN32 
	closesocket(_socketFD);
	WSACleanup();
#else
	close(_socketFD);
#endif
}

bool Socket::isConnected()
{
#ifdef WIN32
	return _socketFD != INVALID_SOCKET;
#else
	int error = 0;
	socklen_t len = sizeof(error);
	int sockopt = getsockopt(_socketFD, SOL_SOCKET, SO_ERROR, &error, &len);
	return error == 0;
#endif
}

void Socket::sendMessage(std::string  message)
{
  // std::cout << "sending: " << message ;

	const char * message_char = message.c_str();
	int len = strlen(message_char);
	int total = 0;        // how many bytes we've sent
	int bytesleft = len;  // how many we have left to send
	int n = 0;
	while (total < len) {

		n = send(_socketFD, (char *)(message_char + total), bytesleft, 0);

		if (n == -1) { break; }
		total += n;
		bytesleft -= n;
	}

  // std::cout << "... done" << std::endl;
}

std::string Socket::receiveMessage()
{
	std::string output;
	char buffer[1024];

	int n;
	errno = 0;
	while (((n = recv(_socketFD, buffer, sizeof(buffer), 0))>0) ||
		errno == EINTR)
	{
		if (n > 0){
			output.append(buffer, n);
		}
		else
		{
			break;
		}
	}

	return output;
}

int Socket::receiveMessage(char *buf, int len) {
	int total = 0;        // how many bytes we've received
	int bytesleft = len; // how many we have left to receive
	int n = 0;
	while (total < len) {
#ifdef WIN32
		n = recv(_socketFD, (char *)(buf + total), bytesleft, 0);
#else
		n = recv(_socketFD, buf + total, bytesleft, 0);
#endif
		//if (n == -1) { break; }
		total += n;
		bytesleft -= n;
	}
	return n == -1 ? -1 : total; // return -1 on failure, total on success
}

cv::Mat Socket::receiveImage()
{
	//receive data
	std::string reply;
	while (reply.empty()){
		reply = receiveMessage();
	}

	//parse data
	std::vector<std::string> strings;
	std::string::size_type prev_pos = 0, pos = 0;

	pos = reply.find('\n', pos);
	strings.push_back(reply.substr(prev_pos, pos - prev_pos));
	prev_pos = ++pos;

	pos = reply.find('\n', pos);
	strings.push_back(reply.substr(prev_pos, pos - prev_pos));
	prev_pos = ++pos;

	strings.push_back(reply.substr(prev_pos, reply.size() - prev_pos));

	int width = atoi(strings[0].substr(22, 4).c_str());
	int height = atoi(strings[0].substr(27, 4).c_str());
	int datasize = atoi(strings[1].c_str());

	//convert endianess
	char* data = new char[datasize];
	char* dataBigEndian = const_cast<char*> (strings[2].c_str());

	int * ptr_org = (int*)dataBigEndian;
	int * ptr_dest = (int*)data;

	for (int i = 0; i < datasize / 4; i++, ptr_org++, ptr_dest++)
	{
		*ptr_dest = (*ptr_org << 24) |
			((*ptr_org << 8) & 0x00ff0000) |
			((*ptr_org >> 8) & 0x0000ff00) |
			((*ptr_org >> 24) & 0x000000ff);
	}

	cv::Mat image(cv::Size(width, height), CV_32FC1, (float*)data); 
	//we clone so that we can delete the data array
	cv::Mat out_image = image.clone();

	//delete data
	delete[] data;

	return out_image;
}

void Socket::receiveImageData(int depth, float* data)
{
	int datasize = 4 * 2048 * 2048;

	std::string message = "STREAM_RECONSTRUCTION " + std::to_string(depth) + "\n0\n";
	if (DEBUG) std::cout << "Send" << message << std::endl;
	sendMessage(message);

	//receive data
	std::string expectedReply = "STREAM_RECONSTRUCTION 2048 2048 " + filename + "  " + std::to_string(depth) + " " + std::to_string(format) + "\n" + std::to_string(datasize) + "\n";
	
	Sleep(500);
	receiveMessage(dummyBuffer, expectedReply.size());
	if (DEBUG) std::cout << "dummyBuffer:" << dummyBuffer << std::endl;
	std::string reply = std::string(dummyBuffer, expectedReply.size());
	if (reply != expectedReply) {
	  std::cout << "REPLY:" << reply << ":vs XPCT:" << expectedReply << std::endl;
	  assert(reply == expectedReply);
	}
	if (DEBUG) std::cout << "Receive " << reply << std::endl;

	char * dataPtr = (char *) &data[0];
	datasize = receiveMessage(dataPtr, datasize);

	int * ptr_org = (int*) &data[0];
	int * ptr_dest = (int*) &data[0];

	for (int i = 0; i < datasize / 4; i++, ptr_org++, ptr_dest++)
	{
		*ptr_dest = (*ptr_org << 24) |
			((*ptr_org << 8) & 0x00ff0000) |
			((*ptr_org >> 8) & 0x0000ff00) |
			((*ptr_org >> 24) & 0x000000ff);
	}
}

bool Socket::setOutputMode(int mode)
{
	format = mode;
	std::string message = "OUTPUT_MODE " + std::to_string(mode) + "\n0\n";
	sendMessage(message);

	receiveMessage(&dummyBuffer[0], message.size());

	std::string reply = std::string(dummyBuffer, message.size());

	if (DEBUG) std::cout << reply << std::endl;
	assert(reply == message);
	if (reply != message) return false;
	Sleep(500);

	return true;
}

bool Socket::setImage(std::string folder, std::string _filename)
{
  	filename = _filename;
	sendMessage("RECONSTRUCT_HOLOGRAMS " + folder + filename + "\n0\n");

	Sleep(10);
	receiveMessage(&dummyBuffer[0], 26);

	std::string reply = std::string(dummyBuffer,26);
	if (reply != "RECONSTRUCT_HOLOGRAMS 1\n0\n") {
	  std::cout << "REPLY:" << reply << ", expect:" << "RECONSTRUCT_HOLOGRAMS 1\n0\n" << std::endl;
	  assert(reply == "RECONSTRUCT_HOLOGRAMS 1\n0\n");
	}

	if (reply != "RECONSTRUCT_HOLOGRAMS 1\n0\n") return false;

	if (DEBUG) std::cout << reply << std::endl;
	return true;
}
