//
// Created by PinkySmile on 19/06/25.
//

#ifndef POKEAI_EMULATORGAMEHANDLE_HPP
#define POKEAI_EMULATORGAMEHANDLE_HPP


#include <memory>
#include <map>
#include "GameEngine/Gen1/State.hpp"
#include "EmulatorHandle.hpp"

#define PING_BYTE 0x60
#define SYNC_BYTE 0xFD
#define UNAVAILABLE_BYTE 0xFE

// These are just things the game sent me when I was testing
// I have no idea what those value does
#define PACKET_FOOTER std::vector<unsigned char>{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x3A, 0x3E, 0x47, 0x51, 0x5E, 0x66, 0x6A, 0x73, 0x7D, 0x8A, 0x92, 0x96, 0x9F, 0xA9, 0xB6, 0xBE, 0xC2, 0xCB, 0xD5, 0xE2, 0xEA, 0xEE, 0xF7, 0xFF, 0x05, 0xFF}

#define CHAR_INVALID 0xFF

#define CHAR_ARR_DOW 0xEF
#define CHAR_ARR_BLA 0xF0
#define CHAR_ARR_WHI 0xF1
#define CHAR_AP_M 0xF2
#define CHAR_AP_R 0xF3
#define CHAR_AP_V 0xF4
#define CHAR_AP_T 0xF5
#define CHAR_AP_S 0xF6
#define CHAR_AP_L 0xF7
#define CHAR_AP_D 0xF8
#define CHAR_E_ACCENT 0xF9
#define CHAR_FEM_NUM 0xFA
#define CHAR_MAL_NUM 0xFB
#define CHAR_MN_NUM  0xFC
#define CHAR_PK_NUM  0xFD
#define CHAR_TRAINER 0xFE

namespace PokemonGen1
{
	class EmulatorGameHandle {
	public:
		typedef std::function<void (const std::string &message)> Logger;

		enum Gen1ConnectionStage
		{
			PKMN_CENTER,
			PINGING_OPPONENT,
			ROOM_CHOOSE,
			PING_POKEMON_EXCHANGE,
			EXCHANGE_POKEMONS,
			BATTLE,
		};

		typedef std::function<EmulatorHandle *(
			const ByteHandle &byteHandle,
			const LoopHandle &loopHandle
		)> EmulatorCreator;

	private:
		static void _displayPacket(std::vector<unsigned char> packet);
		static const std::map<char, const char *> _dispSpecialChars;

		bool _ready = false;
		bool _logMsg;
		bool _isPlayer2;
		bool _done = false;
		bool _sent = false;
		unsigned char _buffer = 0;
		unsigned char _syncSignalsSent = 0;
		unsigned char _syncSignalsReceived = 0;
		unsigned int _timer = 0;
		unsigned int _val = 0;

		BattleState &_state;
		ByteHandle _byteHandler = [this](EmulatorHandle &handle, unsigned char byte) {
			return this->_handleReceivedBytes(handle, byte);
		};
		LoopHandle _loopHandle = [this](EmulatorHandle &handle){
			this->_mainLoop(handle);
		};
		std::unique_ptr<EmulatorHandle> _emulator;
		Gen1ConnectionStage _stage = PKMN_CENTER;
		std::vector<unsigned char> _receiveBuffer;
		std::vector<unsigned char> _last;
		std::pair<unsigned, unsigned> _sendBufferIndex = {0, 0};
		std::vector<std::vector<unsigned char>>	_sendBuffer;

		std::vector<std::vector<unsigned char>> _craftPacket(const BattleState &state);
		void _interpretPacket();
		unsigned char _handleReceivedBytes(EmulatorHandle &handle, unsigned char byte);
		void _mainLoop(EmulatorHandle &handle);
		void _log(const std::string &msg) const;

	public:
		EmulatorGameHandle(
			const EmulatorCreator &emulatorMaker,
			BattleState &state,
			bool player2 = false,
			bool log = false
		);

		void setReady(bool ready);
		Gen1ConnectionStage getStage() const;
		void setStage(Gen1ConnectionStage stage);
		bool isConnected() const;
		bool isReady() const;
		std::pair<unsigned, unsigned> getBattleSendingProgress() const;

		static std::vector<unsigned char> convertString(const std::string &str);
		static std::string convertString(const std::vector<unsigned char> &str);
	};

	extern const std::vector<unsigned char> ASCIIToPkmn1CharConversionTable;
	extern const std::vector<unsigned char> Pkmn1CharToASCIIConversionTable;
}


#endif //POKEAI_EMULATORGAMEHANDLE_HPP
