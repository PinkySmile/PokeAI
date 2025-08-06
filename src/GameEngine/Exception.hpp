//
// Created by Gegel85 on 20/06/2019.
//

#ifndef POKE_AI_EXCEPTION_HPP
#define POKE_AI_EXCEPTION_HPP


#include <exception>
#include <string>

class BaseException : public std::exception {
private:
	std::string _msg;

public:
	explicit BaseException(const std::string &msg) : _msg(msg) {};
	const char *what() const noexcept override { return this->_msg.c_str(); };
};

class EOFException : public BaseException {
public:
	explicit EOFException(const std::string &msg) : BaseException(msg) {};
};

class SocketCreationErrorException : public BaseException {
public:
	explicit SocketCreationErrorException(const std::string &msg) : BaseException((msg)) {};
};

class NotConnectedException : public BaseException {
public:
	explicit NotConnectedException(const std::string &msg) : BaseException(msg) {};
};

class DisconnectException : public BaseException {
public:
	explicit DisconnectException(const std::string &msg) : BaseException(msg) {};
};

class InvalidVersionException : public BaseException {
public:
	explicit InvalidVersionException(const std::string &msg) : BaseException(msg) {};
};

class AlreadyOpenedException : public BaseException {
public:
	explicit AlreadyOpenedException(const std::string &msg) : BaseException(msg) {};
};

class ConnectException : public BaseException {
public:
	explicit ConnectException(const std::string &msg) : BaseException(msg) {};
};

class HostNotFoundException : public BaseException {
public:
	explicit HostNotFoundException(const std::string &msg) : BaseException(msg) {};
};

class TimeOutException : public BaseException {
public:
	explicit TimeOutException(const std::string &msg) : BaseException(msg) {};
};

class UnexpectedUserActionException : public BaseException {
public:
	explicit UnexpectedUserActionException(const std::string &msg) : BaseException(msg) {};
};

class NotImplementedException : public BaseException {
public:
	explicit NotImplementedException(const std::string &msg) : BaseException(msg) {};
};

class InvalidStateException : public BaseException {
public:
	explicit InvalidStateException(const std::string &msg) : BaseException(msg) {};
};

class InvalidSaveFileException : public BaseException {
public:
	explicit InvalidSaveFileException(const std::string &msg) : BaseException(msg) {};
};

class DesyncException : public BaseException {
public:
	explicit DesyncException(const std::string &msg) : BaseException(msg) {};
};

class OpponentCrashedException : public BaseException {
public:
	explicit OpponentCrashedException(const std::string &msg) : BaseException(msg) {};
};


#endif //POKE_AI_EXCEPTION_HPP
