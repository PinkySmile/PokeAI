//
// Created by PinkySmile on 13/07/2019.
//

#ifndef POKEAI_BATTLEHANDLER_HPP
#define POKEAI_BATTLEHANDLER_HPP


#include <memory>
#include <map>
#include <deque>
#include "Pokemon.hpp"
#include "RandomGenerator.hpp"
#include "State.hpp"

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
	class BattleHandler {
	public:
		struct ReplayData {
			std::string nameP1;
			std::string nameP2;
			std::vector<Pokemon> teamP1;
			std::vector<Pokemon> teamP2;
			std::vector<unsigned char> rngList;
			std::vector<std::pair<BattleAction, BattleAction>> input;
		};

		BattleHandler(bool viewSwapped, bool logMessages);

		void logBattle(const std::string &message);
		BattleState &getBattleState();
		const BattleState &getBattleState() const;
		bool tick();
		bool isFinished() const;
		void reset();
		bool saveReplay(const std::string &path);
		void loadReplay(const std::string &path);
		bool saveState(const std::string &path);
		bool loadState(const std::string &path);
		void stopReplay();
		bool playingReplay() const;
		void start();

	private:
		ReplayData _replayData;
		BattleState _state;
		std::deque<std::pair<BattleAction, BattleAction>> _replayInputs;
		bool _playingReplay = false;
		bool _isViewSwapped;
		bool _started = false;
		bool _finished = false;
		bool _logMessages;

		void _checkAction(PlayerState &myState, const PlayerState &opState);
		void _checkActions();
		void _populateStartParams();
		void _executeBattleActions();
		void _makePlayersAttack(bool AIAttack, bool opponentAttack);
		void _log(const std::string &msg);
	};
}


#endif //POKEAI_BATTLEHANDLER_HPP
