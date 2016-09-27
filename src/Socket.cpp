#include "Socket.h"
#include <iostream>

#define MAX_FLUSH_CYCLE 10
#define OCTOTERM "\n0\n"

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
	int iTimeout = 300;
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

void Socket::receiveImageData(int depth, float* data)
{
	std::vector<std::string> strings;
	int datasize;

	std::string message = "STREAM_RECONSTRUCTION " + std::to_string(depth) + OCTOTERM;
	bool dataReady = false;
	
	sendMessage(message);

	//receive data
	std::string reply;
		
	while (strings.size() <=2 || datasize > strings[2].size())
	{
		strings.clear();
		std::string reply2;
			
		while (reply2.empty()){
			reply2 = receiveMessage();
			reply += reply2;
		}

		// Parse data.  The problem is that sometimes a reply
		// has a "RECONSTRUCT_HOLOGRAMS 1\n0\n" prefixed to it.
		// So the parsing is careful.
		std::string::size_type prev_pos = 0, pos = 0;

		while (reply.substr(pos, 6).compare("STREAM") != 0) {
		  if (reply.size() < 300) {
		    std::cout << "here's the reply we don't want:" << reply << "<<" << std::endl;
		  }

		  reply.clear();
		  reply2.clear();
		  while (reply2.empty()){
		    reply2 = receiveMessage();
		    reply += reply2;
		  }
		}

		// This should pick up the "STREAM_RECONSTRUCTION..." line.
		pos = reply.find('\n', pos);
		strings.push_back(reply.substr(prev_pos, pos - prev_pos));
		prev_pos = ++pos;

		// This should get the N, which is usually 16777216.
		pos = reply.find('\n', pos);
		strings.push_back(reply.substr(prev_pos, pos - prev_pos));
		prev_pos = ++pos;

		strings.push_back(reply.substr(prev_pos, reply.size() - prev_pos));

		// Parse the important values from the first line.
		pos = 0;
		prev_pos = 0;
		std::vector<std::string> stringvals;
		pos = strings[0].find(' ', pos);
		while (pos != std::string::npos) {
		  stringvals.push_back(strings[0].substr(prev_pos, pos-prev_pos));

		  prev_pos = ++pos;
		  pos = strings[0].find(' ', pos);
		}

		// Parse the various values from the header line.
		int width = atoi(stringvals[1].c_str());
		int height = atoi(stringvals[2].c_str());
		std::string filename = stringvals[3];
		int level = atoi(stringvals[3].c_str());
		int reconType = atoi(strings[0].substr(prev_pos).c_str());

		// Parse the N from the second line.
		datasize = atoi(strings[1].c_str());

		// Everything else received should be the data, so it should
		// all be in strings[2].
	}

	//convert endianess
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
}

void Socket::setOutputMode(int mode)
{
	// //flush leftover
	// int count = 0;
	// while (count < MAX_FLUSH_CYCLE){
	// 	receiveMessage();
	// 	count++;
	// }

	//set to phase images
	sendMessage("OUTPUT_MODE " + std::to_string(mode) + OCTOTERM);
	// std::string reply;
	// while (reply.empty()){
	// 	reply = receiveMessage();
	// }
	// if (reply.size() < 100){
	// 	std::cout << reply << std::endl;
	// }
	// else
	// {
	// 	std::cout << "Discard old data " << reply.size() << std::endl;
	// }

	// //flush receive
	// count = 0;
	// while (count < MAX_FLUSH_CYCLE){
	// 	receiveMessage();
	// 	count++;
	// }
}

void Socket::setImage(std::string filename)
{
	sendMessage("RECONSTRUCT_HOLOGRAMS " + filename + OCTOTERM);

	// std::string reply;
	// while (reply.empty()){
	// 	reply = receiveMessage();
	// }

	// if (reply.size() < 100){
	// 	std::cout << reply << std::endl;
	// }
	// else
	// {
	// 	std::cout << "Discard old data " << reply.size() << std::endl;
	// }
}
