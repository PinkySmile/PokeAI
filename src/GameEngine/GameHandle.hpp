//
// Created by Gegel85 on 13/07/2019.
//

#ifndef POKEAI_GAMEHANDLE_HPP
#define POKEAI_GAMEHANDLE_HPP


#include <memory>
#include <map>
#include "Pokemon.hpp"
#include "PokemonRandomGenerator.hpp"
#include "../Networking/EmulatorHandle.hpp"

#define PING_BYTE 0x60
#define SYNC_BYTE 0xFD
#define UNAVAILABLE_BYTE 0xFE

//These are just things the game sent me when I was testing
//I have no idea what those value does
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
	const std::map<char, const char *> _dispSpecialChars{
		{ CHAR_E_ACCENT, "é" },
		{ CHAR_AP_D, "'d" },
		{ CHAR_AP_L, "'l" },
		{ CHAR_AP_S, "'s" },
		{ CHAR_AP_T, "'t" },
		{ CHAR_AP_V, "'v" },
		{ CHAR_AP_R, "'r" },
		{ CHAR_AP_M, "'m" },
		{ CHAR_ARR_WHI, "→" },
		{ CHAR_ARR_BLA, "►" },
		{ CHAR_ARR_DOW, "▼" },
		{ CHAR_TRAINER, "<TRAINER>"},
		{ CHAR_PK_NUM, "pₖ" },
		{ CHAR_MN_NUM, "mₙ" },
		{ CHAR_MAL_NUM, "♂" },
		{ CHAR_FEM_NUM, "♀" }
	};

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
		StruggleMove = 0x6E,
		Run,
	};

	struct BattleState {
		std::string		name;
		std::string		opponentName;
		unsigned		pokemonOnField;
		unsigned		opponentPokemonOnField;
		std::vector<Pokemon>	team;
		std::vector<Pokemon>	opponentTeam;
		BattleAction		lastAction;
		BattleAction		nextAction;
		BattleAction		nextOpponentAction;
	};

	typedef std::function<void (const std::string &message)> Logger;
	void displayPacket(std::vector<unsigned char> packet);

	class GameHandle {
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
		bool					_ready = true;
		bool					_logMsg;
		std::unique_ptr<EmulatorHandle>		_emulator;
		std::string				_trainerName;
		bool					_isPlayer2;
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
		bool					_sent = false;
		std::pair<unsigned, unsigned>		_sendBufferIndex = {0, 0};
		std::vector<std::vector<unsigned char>>	_sendBuffer;
		Logger					_battleLogger;
		std::function<BattleAction(GameHandle &)> _battleHandler;
		std::function<std::pair<BattleAction,BattleAction>(GameHandle &)> _battleHandler2;

		std::vector<std::vector<unsigned char>> _craftPacket();
		void _interpretPacket();
		void _executeBattleActions();
		unsigned char _handleReceivedBytes(EmulatorHandle &handle, unsigned char byte);
		void _mainLoop(EmulatorHandle &handle);
		void _makePlayersAttack(bool AIAttack, bool opponentAttack);

		void _log(const std::string &msg);

	public:
		explicit GameHandle(
			const std::function<
				EmulatorHandle *(
					const ByteHandle &byteHandle,
					const LoopHandle &loopHandle,
					const std::string &ip,
					unsigned short port
				)
			> &emulatorMaker,
			const std::function<BattleAction(GameHandle &)> &battleHandler,
			const std::string &trainerName = "PokeAI",
			const Logger &battleLogger = {},
			bool player2 = false,
			bool log = false
		);
		explicit GameHandle(
			const std::function<std::pair<BattleAction, BattleAction>(GameHandle &)> &battleHandler,
			const std::pair<std::string, std::string> &trainerNames = { "PokeAI", "PokeAI" },
			const Logger &battleLogger = {},
			bool log = false
		);

		template <typename ...types>
		void addPokemonToTeam(types ...args)
		{
			if (this->_pkmns.size() >= 6)
				throw std::out_of_range("There are already 6 pokémons in the team");
			this->_pkmns.emplace_back(this->_randomGenerator, *this, args...);
		}

		template <typename ...types>
		void changePokemon(unsigned char index, types ...args)
		{
			std::vector<Pokemon> buffer;

			if (this->_pkmns.size() <= index)
				throw std::out_of_range("Out of range");

			for (const Pokemon &pkmn : this->_pkmns) {
				if (buffer.size() == index)
					buffer.emplace_back(this->_randomGenerator, *this, args...);
				else
					buffer.push_back(pkmn);
			}

			this->_pkmns.clear();
			for (const Pokemon &pkmn : buffer)
				this->_pkmns.push_back(pkmn);
		}

		void connect(const std::string &ip, unsigned short port);
		void disconnect();
		void setReady(bool ready);

		Gen1ConnectionStage getStage() const;
		void logBattle(const std::string &message);
		bool isConnected() const;
		bool isReady() const;
		void setTeamSize(unsigned size);
		void deletePokemon(unsigned char index);
		Pokemon &getPokemon(unsigned index);
		const std::vector<Pokemon> &getPokemonTeam() const;
		const BattleState &getBattleState() const;
		const std::string &getTrainerName() const;
		std::pair<unsigned, unsigned> getBattleSendingProgress();
		void setTrainerName(const std::string &trainerName);
		std::vector<unsigned char> save();
		void load(const std::vector<unsigned char> &data);

		static std::vector<unsigned char> convertString(const std::string &str);
		static std::string convertString(const std::vector<unsigned char> &str);
	};

	extern const std::vector<unsigned char> ASCIIToPkmn1CharConversionTable;
	extern const std::vector<unsigned char> Pkmn1CharToASCIIConversionTable;
}


#endif //POKEAI_GAMEHANDLE_HPP
