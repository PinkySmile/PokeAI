//
// Created by Gegel85 on 13/07/2019.
//

#ifndef POKEAI_PKMNGEN1HANDLE_HPP
#define POKEAI_PKMNGEN1HANDLE_HPP


#include <memory>
#include "Pokemon.hpp"
#include "PokemonRandomGenerator.hpp"
#include "EmulatorHandle.hpp"

#define PING_BYTE 0x60
#define SYNC_BYTE 0xFD

//These are just things the game sent me when I was testing
//I have no idea what those value does
#define NO_PKMN std::vector<unsigned char>{0x50, 0x8D, 0x84, 0x54, 0x00, 0xFF, 0x31, 0x00, 0x17, 0x17, 0xA3, 0x62, 0x15, 0xFF, 0x55, 0x43, 0x1A, 0xFF, 0xD2, 0xE4, 0x1E, 0x6E, 0x22, 0xC9, 0x20, 0xC4, 0xFF, 0x1E, 0x1A, 0x9F, 0x7F, 0x7D, 0x1E, 0x14, 0x14, 0x0F, 0xFF, 0x6F, 0xA4, 0x95, 0x16, 0x69, 0x68, 0x67}
#define HEADER_PACKET std::vector<unsigned char>{0xFE, 0xCE, 0xB2, 0x98, 0x80, 0x6A, 0x56, 0x43, 0xFE, 0x32, 0x23, 0x16}
#define MIDDLE_PACKET std::vector<unsigned char>{0xE5, 0xC9, 0xD3, 0xFE, 0xB2, 0xFE, 0x1E, 0xFE, 0xAA, 0xFE, 0x37, 0xE5, 0xC9, 0xD3, 0xFE, 0xB2, 0xFE, 0x1E, 0xFE, 0xAA, 0xFE, 0x37, 0xE5, 0xC9, 0xD3, 0xFE, 0xB2, 0xFE, 0x1E, 0xFE, 0xAA, 0xFE, 0x37, 0xE5, 0xC9, 0xD3, 0xFE, 0xB2, 0xFE, 0x1E, 0xFE, 0xAA, 0xFE, 0x37, 0xE5, 0xC9, 0xD3, 0xFE, 0xB2, 0xFE, 0x1E, 0xFE, 0xAA, 0xFE, 0x37, 0x8F, 0x88, 0x8A, 0x80, 0x82, 0x87, 0x94, 0x50, 0x50, 0x50, 0x50, 0x50, 0x8D, 0x84, 0x54, 0x00, 0xFE, 0x31, 0x00, 0x17, 0x17, 0xA3, 0x50}
#define PACKET_FOOTER std::vector<unsigned char>{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x3A, 0x3E, 0x47, 0x51, 0x5E, 0x66, 0x6A, 0x73, 0x7D, 0x8A, 0x92, 0x96, 0x9F, 0xA9, 0xB6, 0xBE, 0xC2, 0xCB, 0xD5, 0xE2, 0xEA, 0xEE, 0xF7, 0xFF, 0x05, 0xFF}

#define CHAR_INVALID 0xFF
#define CHAR_TRAINER '&'
#define CHAR_PK_NUM  '\''
#define CHAR_MN_NUM  '"'
#define CHAR_MAL_NUM '|'
#define CHAR_FEM_NUM '#'

namespace Pokemon
{
	enum Gen1ConnectionStage
	{
		PKMN_CENTER,
		PINGING_OPPONENT,
		ROOM_CHOOSE,
		PING_POKEMON_EXCHANGE,
		EXCHANGE_POKEMONS,
		BATTLE,
	};

	enum BattleAction {
		NoAction,
		Attack1 = 0x60,
		Attack2,
		Attack3,
		Attack4,
		Switch1,
		Switch2,
		Switch3,
		Switch4,
		Switch5,
		Switch6,
		Run = 0x6F,
	};

	struct BattleState {
		std::string		opponentName;
		unsigned		pokemonOnField;
		unsigned		opponentPokemonOnField;
		std::vector<Pokemon>	team;
		std::vector<Pokemon>	opponentTeam;
		BattleAction		nextAction;
		BattleAction		nextOpponentAction;
	};

	void displayPacket(std::vector<unsigned char> packet);

	class PkmnGen1Handle {
	private:
		BattleState	_state;
		ByteHandle	_byteHandler;
		std::function<
			EmulatorHandle *(
				const ByteHandle &byteHandle,
				const LoopHandle &loopHandle,
				const std::string &ip,
				unsigned short port
			)
		>					_emulatorMaker;
		std::unique_ptr<EmulatorHandle>		_emulator;
		std::string				_trainerName;
		bool					_isPlayer2;
		char					_received = false;
		PokemonRandomGenerator			_randomGenerator;
		Gen1ConnectionStage			_stage = PKMN_CENTER;
		bool					_done = false;
		unsigned int				_timer = 0;
		unsigned char				_buffer = 0;
		unsigned char				_syncSignalsSent = 0;
		unsigned char				_syncSignalsReceived = 0;
		std::vector<Pokemon>			_pkmns;
		std::vector<unsigned char>		_receiveBuffer;
		std::vector<unsigned char>		_last;
		unsigned				_controlSignalsRecieved;
		bool					_sent = false;
		std::pair<unsigned, unsigned>		_sendBufferIndex = {0, 0};
		std::vector<std::vector<unsigned char>>	_sendBuffer;
		std::function<BattleAction(PkmnGen1Handle &)> _battleHandler;

		std::vector<std::vector<unsigned char>> _craftPacket();
		void _interpretPacket();
		void _executeBattleActions();
		unsigned char _handleReceivedBytes(EmulatorHandle &handle, unsigned char byte);
		void _mainLoop(EmulatorHandle &handle);

		void _log(const std::string &msg);

	public:
		explicit PkmnGen1Handle(
			const std::function<
				EmulatorHandle *(
					const ByteHandle &byteHandle,
					const std::string &ip,
					unsigned short port
				)
			> &emulatorMaker,
			const std::function<BattleAction(PkmnGen1Handle &)> &battleHandler,
			const std::string &trainerName = "PokeAI",
			bool player2 = false
		);

		template <typename ...types>
		void addPokemonToTeam(types ...args)
		{
			this->_pkmns.emplace_back(this->_randomGenerator, args...);
		}

		void connect(const std::string &ip, unsigned short port);

		std::vector<unsigned char> convertString(const std::string &str);
		static std::string convertString(const std::vector<unsigned char> &str);
	};

	extern const std::vector<unsigned char> ASCIIToPkmn1CharConversionTable;
	extern const std::vector<unsigned char> Pkmn1CharToASCIIConversionTable;
}


#endif //POKEAI_PKMNGEN1HANDLE_HPP
