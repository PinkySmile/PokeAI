//
// Created by Gegel85 on 05/04/2019.
//

#ifndef DISC_ORD_SOCKET_HPP
#define DISC_ORD_SOCKET_HPP


#if defined _WIN32
#	include <winsock.h>
#else
#	include <sys/socket.h>
#	define INVALID_SOCKET -1
	typedef int SOCKET;
#endif
#include <string>
#include <map>

class Socket {
public:
	struct HttpRequestIn {
		std::string				body;
		std::string				method;
		std::string				host;
		int					portno;
		std::map<std::string, std::string>	header;
		std::string				path;
	};

	struct HttpRequestOut {
		std::map<std::string, std::string>	header;
		int					returnCode;
		std::string				codeName;
		std::string				httpVer;
		std::string				body;
	};

	static std::string getLastSocketError();

	Socket();
	Socket(SOCKET sock, bool connected = false);
	~Socket();
	bool			isOpen();
	virtual void		connect(const std::string &host, unsigned short portno);
	virtual void		connect(unsigned int ip, unsigned short portno);
	virtual void		disconnect();
	virtual void		send(const std::string &);
	virtual std::string	read(int size);
	virtual std::string	readUntilEOF();
	static std::string	generateHttpRequest(const HttpRequestIn &);
	HttpRequestOut		makeHttpRequest(const HttpRequestIn &);
	unsigned		getRemoteIp();
	std::string		makeRawRequest(const std::string &host, unsigned short portno, const std::string &content);
	SOCKET			getSockFd() const { return this->_sockfd; };

protected:
	SOCKET		_sockfd = INVALID_SOCKET;
	bool		_opened = false;
	unsigned	_remote = 0;

	HttpRequestOut	_parseHttpResponse(const std::string &);
};

#endif //DISC_ORD_SOCKET_HPP
