//
// Created by Gegel85 on 13/07/2019.
//

#ifndef POKEAI_BGBHANDLER_HPP
#define POKEAI_BGBHANDLER_HPP


#include <vector>
#include <thread>
#include <iostream>
#include <functional>
#include "Socket.hpp"
#include "EmulatorHandle.hpp"

class BGBHandler : public EmulatorHandle {
private:
	bool _logging = false;
	Socket _socket;
	std::thread _mainThread;
	unsigned int _ticks = 0;
	std::function<void ()> _mainHandler;

	struct BGBPacket {
		unsigned char b1;
		unsigned char b2;
		unsigned char b3;
		unsigned char b4;
		unsigned int  i1;
	};

	bool _handleLoop();
	void _sendPacket(const BGBPacket &packet);
	void _sync();
	BGBPacket _getNextPacket();

public:
	BGBHandler(
		const std::function<unsigned char (EmulatorHandle &handler, unsigned char byte)> &masterHandler,
		const std::function<unsigned char (EmulatorHandle &handler, unsigned char byte)> &slaveHandler,
		const std::string &ip,
		unsigned short port,
		bool log = false
	);
	void log(const std::string &string, std::ostream &stream = std::cout) override;
	void disconnect() override;
	void sendByte(unsigned char byte) override;
	void reply(unsigned char byte) override;
	bool isConnected();
	~BGBHandler();

	enum BGBCommand {
		VERSION_CHECK	= 1,
		JOYPAD_CHANGE	= 101,
		SYNC1_SIGNAL	= 104,
		SYNC2_SIGNAL	= 105,
		SYNC3_SIGNAL	= 106,
		STATUS		= 108,
		WANT_DISCONNECT	= 109,
	};

	enum BGBStatusFlag {
		STATUSFLAG_RUNNING		= 1U << 0U,
		STATUSFLAG_PAUSED		= 1U << 1U,
		STATUSFLAG_SUPPORT_RECONNECT	= 1U << 2U,
	};

	static const char PACKET_SIZE = 8;
};


#endif //POKEAI_BGBHANDLER_HPP
