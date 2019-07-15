//
// Created by Gegel85 on 13/07/2019.
//

#ifndef POKEAI_EMULATORHANDLE_HPP
#define POKEAI_EMULATORHANDLE_HPP


#include <iostream>
#include <functional>

class EmulatorHandle {
protected:
	bool _log;
	bool _disconnected = true;
	std::function<unsigned char (EmulatorHandle &handler, unsigned char byte)> _slaveHandler;
	std::function<unsigned char (EmulatorHandle &handler, unsigned char byte)> _masterHandler;

	EmulatorHandle(
		const std::function<unsigned char (EmulatorHandle &handler, unsigned char byte)> &masterHandler,
		const std::function<unsigned char (EmulatorHandle &handler, unsigned char byte)> &slaveHandler,
		bool log
	) :
		_log(log),
		_slaveHandler(slaveHandler),
		_masterHandler(masterHandler)
	{};

public:
	virtual void log(const std::string &string, std::ostream &stream = std::cout) = 0;
	virtual void disconnect() = 0;
	virtual void sendByte(unsigned char byte) = 0;
	virtual void reply(unsigned char byte) = 0;
};


#endif //POKEAI_EMULATORHANDLE_HPP
