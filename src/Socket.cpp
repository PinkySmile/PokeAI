//
// Created by Gegel85 on 05/04/2019.
//

#include <cstring>
#include <sstream>
#include <unistd.h>
#include "Socket.hpp"
#include "Exception.hpp"

#ifndef _WIN32
#	include <netdb.h>
#	include <arpa/inet.h>
#	include <sys/select.h>
	typedef fd_set FD_SET;
#endif

#ifndef closesocket
#define closesocket(socket) close(socket)
#endif

#include <iostream>

std::string Socket::getLastSocketError()
{
#ifdef _WIN32
	wchar_t *s = nullptr;

	FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&s,
		0,
		NULL
	);

	std::stringstream stream;

	stream << "WSAGetLastError " << WSAGetLastError() << ": ";
	for (int i = 0; s[i]; i++)
		stream << static_cast<char>(s[i]);
	LocalFree(s);
	return stream.str();
#else
	return strerror(errno);
#endif
}

Socket::Socket()
{
#ifdef _WIN32
	WSADATA			WSAData;
	WSAStartup(MAKEWORD(2,0), &WSAData);
#endif

	/* create the socket */
	this->_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (this->_sockfd == INVALID_SOCKET)
		throw SocketCreationErrorException(getLastSocketError());
}

Socket::Socket(SOCKET sock, bool connected)
{
	this->_sockfd = sock;
	this->_opened = connected;
}

Socket::~Socket()
{
	closesocket(this->_sockfd);
}

void Socket::connect(const std::string &host, unsigned short portno)
{
	struct hostent	*server;

	if (this->isOpen())
		throw AlreadyOpenedException("This socket is already opened");

	/* lookup the ip address */
	server = gethostbyname(host.c_str());
	if (server == nullptr)
		throw HostNotFoundException("Cannot find host '" + host + "'");
	this->connect(*reinterpret_cast<unsigned *>(server->h_addr), portno);
}

void Socket::connect(unsigned int ip, unsigned short portno)
{
	struct sockaddr_in	serv_addr = {};

	if (this->isOpen())
		throw AlreadyOpenedException("This socket is already opened");

	/* fill in the structure */
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = ip;

	/* connect the socket */
	if (::connect(this->_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		throw ConnectException(std::string("Cannot connect to ") + inet_ntoa(serv_addr.sin_addr) + " on port " + std::to_string(portno));
	this->_opened = true;
	this->_remote = ip;
}

Socket::HttpRequestOut Socket::_parseHttpResponse(const std::string &respon)
{
	std::stringstream	response(respon);
	HttpRequestOut		request;
	std::string		str;

	response >> request.httpVer;
	response >> request.returnCode;
	std::getline(response, request.codeName);
	request.codeName = request.codeName.substr(1, request.codeName.length() - 2);
	while (std::getline(response, str) && str.length() > 2) {
		std::size_t	pos = str.find(':');

		request.header[str.substr(0, pos)] = str.substr(pos + 2, str.size() - pos);
	}
	request.body = response.str().substr(response.tellg());
	return request;
}

std::string Socket::generateHttpRequest(const Socket::HttpRequestIn &request)
{
	std::stringstream	msg;

	/* fill in the parameters */
	msg << request.method << " " << request.path << " HTTP/1.0\r\n";
	msg << "Host: " << request.host << "\r\n";
	for (auto &entry : request.header)
		msg << entry.first << ": " << entry.second << "\r\n";
	msg << "\r\n" << request.body;
	return msg.str();
}

Socket::HttpRequestOut Socket::makeHttpRequest(const Socket::HttpRequestIn &request)
{
	if (this->isOpen())
		throw AlreadyOpenedException("This socket is already opened");

	std::string requestString = this->generateHttpRequest(request);
	std::string response = this->makeRawRequest(request.host, request.portno, requestString);

	return this->_parseHttpResponse(response);
}

void Socket::disconnect()
{
	if (!this->isOpen())
		throw NotConnectedException("This socket is not connected to a server");
	closesocket(this->_sockfd);
	this->_opened = false;
	this->_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (this->_sockfd == INVALID_SOCKET)
		throw SocketCreationErrorException(strerror(errno));
}

unsigned Socket::getRemoteIp()
{
	if (!this->isOpen())
		throw NotConnectedException("This socket is not connected to a server");
	return this->_remote;
}

std::string Socket::makeRawRequest(const std::string &host, unsigned short portno, const std::string &content)
{
	unsigned	ip;
	std::string	message;

	ip = inet_addr(host.c_str());
	if (ip != INADDR_NONE)
		this->connect(ip, portno);
	else
		this->connect(host, portno);

	this->send(content);
	message = this->readUntilEOF();
	this->disconnect();

	return message;
}

std::string Socket::read(int size)
{
	std::stringstream	stream;
	char	buffer[1024];

	while (size != 0) {
		int bytes = recv(this->_sockfd, buffer, static_cast<unsigned>(size) >= sizeof(buffer) ? sizeof(buffer) : size, 0);

		if (bytes <= 0)
			throw EOFException(getLastSocketError());
		stream << std::string(buffer, bytes);
		size -= bytes;
		if (bytes < static_cast<int>(sizeof(buffer)))
			break;
	}
	return stream.str();
}

void Socket::send(const std::string &msg)
{
	unsigned	pos = 0;

	while (pos < msg.length()) {
		int bytes = ::send(this->_sockfd, &msg.c_str()[pos], msg.length() - pos, 0);

		if (bytes <= 0)
			throw EOFException(getLastSocketError());
		pos += bytes;
	}
}

std::string Socket::readUntilEOF()
{
	return this->read(-1);
}

bool	Socket::isOpen()
{
	FD_SET	set;
	timeval time = {0, 0};

	FD_ZERO(&set);
	FD_SET(this->_sockfd, &set);
	if (this->_opened && select(0, &set, nullptr, nullptr, &time) == -1)
		this->_opened = false;
	return (this->_opened);
}
