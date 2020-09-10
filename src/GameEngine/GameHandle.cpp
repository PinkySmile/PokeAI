//
// Created by Gegel85 on 13/07/2019.
//

#include <thread>
#include "GameHandle.hpp"
#include "../Exception.hpp"

namespace PokemonGen1
{
	void displayPacket(std::vector<unsigned char> packet)
	{
		for (unsigned int i = 0; i < packet.size(); i += 20) {
			for (unsigned j = 0; j < 20 && j + i < packet.size(); j++)
				printf("%02X ", packet[j + i]);
			for (int j = 0; j < static_cast<int>(i - packet.size() + 20); j++)
				printf("   ");
			for (unsigned j = 0; j < 20 && j + i < packet.size(); j++)
				printf("%c", isprint(packet[j + i]) ? packet[j + i] : '.');
			for (int j = 0; j < static_cast<int>(i - packet.size() + 20); j++)
				printf(" ");
			printf(" ");
			for (unsigned j = 0; j < 20 && j + i < packet.size(); j++)
				printf("%c", isprint(Pkmn1CharToASCIIConversionTable[packet[j + i]])
					     ? Pkmn1CharToASCIIConversionTable[packet[j + i]] : '.');
			printf("\n");
		}
	}

	GameHandle::GameHandle(
		const std::function<
			EmulatorHandle *(
				const ByteHandle &byteHandle,
				const LoopHandle &loopHandle,
				const std::string &ip,
				unsigned short port
			)
		> &emulatorMaker,
		const std::function<BattleAction(GameHandle &)> &battleHandler,
		const std::string &trainerName,
		const Logger &battleLogger,
		bool player2,
		bool log
	) :
		_emulatorMaker(emulatorMaker),
		_logMsg(log),
		_emulator{nullptr},
		_trainerName(trainerName),
		_isPlayer2(player2),
		_randomGenerator(),
		_battleLogger(battleLogger),
		_battleHandler(battleHandler)
	{
		if (player2)
			throw NotImplementedException("Player 2 hasn't been implemented");
		if (this->_trainerName.size() > 10) {
			this->_log("Warning: trainer name is too long");
			this->_trainerName = this->_trainerName.substr(0, 10);
		}
		this->_byteHandler = [this](EmulatorHandle &handle, unsigned char byte) {
			return this->_handleReceivedBytes(handle, byte);
		};
	}

	void GameHandle::logBattle(const std::string &message)
	{
		std::cout << "[BATTLE]: " << message << std::endl;
		if (this->_battleLogger)
			this->_battleLogger(message);
	}

	void GameHandle::setTeamSize(unsigned size)
	{
		if (!size || size > 6)
			throw std::invalid_argument("Team size out of range");

		std::vector<PokemonGen1::Pokemon> temp{this->_pkmns};

		this->_pkmns.clear();
		for (unsigned i = 0; i < size; i++)
			if (i < temp.size())
				this->_pkmns.push_back(temp[i]);
			else
				this->_pkmns.emplace_back(
					this->_randomGenerator,
					*this,
					"",
					1,
					PokemonGen1::pokemonList[PokemonGen1::Rhydon],
					std::vector<PokemonGen1::Move>{
						PokemonGen1::availableMoves[PokemonGen1::Tackle],
						PokemonGen1::availableMoves[PokemonGen1::Tail_Whip]
					}
				);
	}

	const std::vector<Pokemon> &GameHandle::getPokemonTeam() const
	{
		return this->_pkmns;
	}

	const BattleState &GameHandle::getBattleState() const
	{
		return this->_state;
	}

	unsigned char GameHandle::_handleReceivedBytes(EmulatorHandle &handle, unsigned char byte)
	{
		if (this->_last.size() == 8)
			this->_last.erase(this->_last.begin());
		this->_last.push_back(byte);
	START:
		switch (this->_stage) {
		case PKMN_CENTER:
			if (byte == 1) {
				this->_log("Pinging opponent");
				this->_stage = PINGING_OPPONENT;
				handle.reply(1 + this->_isPlayer2);
				handle.reply(0);
				handle.reply(0);
				this->_done = false;
				this->_buffer = 0;
				return static_cast<unsigned char>(96);
			} else if (byte == PING_BYTE) {
				this->_log("Pinging opponent");
				this->_stage = PINGING_OPPONENT;
				this->_done = false;
				return static_cast<unsigned char>(this->_isPlayer2 * PING_BYTE);
			} else if (byte >= 208 && byte != 254) {
				this->_buffer = 0;
				this->_done = false;
				this->_timer = 0;
				this->_log("Choosing room");
				this->_stage = ROOM_CHOOSE;
			}
			break;
		case PINGING_OPPONENT:
			if (this->_buffer < 2) {
				this->_buffer++;
				return byte;
			}
			if (byte >= 208) {
				this->_buffer = 0;
				this->_timer = 0;
				this->_log("Choosing room");
				this->_stage = ROOM_CHOOSE;
			} else if (byte == 0 && this->_done) {
				this->_timer = 5;
				this->_done = false;
				this->_buffer = 0;
				this->_log("Choosing room");
				this->_stage = ROOM_CHOOSE;
			} else if (byte == PING_BYTE) {
				this->_done = true;
				return static_cast<unsigned char>(this->_isPlayer2 * PING_BYTE);
			} else if (byte != 254 && byte) {
				this->_log("Back to PKMN Center");
				this->_stage = PKMN_CENTER;
				this->_timer = 0;
				goto START;
			}
			break;
		case ROOM_CHOOSE:
			if (byte != 254 && byte)
				this->_done = true;
			if (byte == 254) {
				if (this->_buffer >= 6) {
					this->_log("Waiting for user to start battle");
					this->_stage = PING_POKEMON_EXCHANGE;
					this->_done = false;
					return static_cast<unsigned char>(254);
				}
				this->_buffer = 0;
			} else if (byte == 2) {
				this->_log("Back to PKMN Center");
				this->_stage = PKMN_CENTER;
				this->_timer = 0;
			} else if (byte == 213) {
				this->_log("User chose colosseum for me");
				this->_stage = PING_POKEMON_EXCHANGE;
				this->_done = false;
			} else if (byte == 216) {
				this->_log("Back to PKMN Center");
				this->_stage = PKMN_CENTER; //User clicked cancel
			} else if (byte >= 212) {
				throw UnexpectedUserActionException("User didn't chose colosseum ( bad user >:( )");
			} else if (byte >= 208) {
				return 208;
			} else if (byte == PING_BYTE) {
				this->_stage = PINGING_OPPONENT;
				this->_log("Oops, seems like we went to far. Going back to pinging the opponent");
				return static_cast<unsigned char>(this->_isPlayer2 * PING_BYTE);
			}
			break;
		case PING_POKEMON_EXCHANGE:
			if (byte == 253 || (byte == 254 && this->_isPlayer2)) {
				this->_sendBuffer = this->_craftPacket();
				this->_sendBufferIndex = {0, 0};
				this->_sent = this->_done = false;
				this->_log("Sending battle data");
				this->_stage = EXCHANGE_POKEMONS;
				this->_receiveBuffer.clear();
			} else if (byte >= 208 && byte != 254){
				this->_log("Oops, we are still choosing room");
				this->_timer = 5;
				this->_done = false;
				this->_buffer = 0;
				this->_stage = ROOM_CHOOSE;
			}
			break;
		case EXCHANGE_POKEMONS:
			this->_receiveBuffer.push_back(byte);
			this->_sent = false;
			this->_done = true;
			if (byte == SYNC_BYTE || byte == UNAVAILABLE_BYTE)
				this->_syncSignalsReceived++;
			else
				this->_syncSignalsReceived = 0;
			if (
				this->_syncSignalsReceived >= 9 - (2 * this->_sendBufferIndex.first) &&
				this->_syncSignalsSent >= 9 - (2 * this->_sendBufferIndex.first)
			) {
				this->_sendBufferIndex.first++;
				this->_sendBufferIndex.second = 0;
			}
			for (unsigned i = 0; i < 7; i++)
				this->_done &= !this->_last[i];
			if (this->_done && byte == UNAVAILABLE_BYTE) {
				this->_interpretPacket();
				this->_stage = BATTLE;
				this->_done = false;
				this->_syncSignalsReceived = 0;
				this->_state.pokemonOnField = 0;
				this->_state.opponentPokemonOnField = 0;
				this->_state.team.clear();
				for (Pokemon &pkmn : this->_pkmns)
					this->_state.team.emplace_back(pkmn);
				this->_state.lastAction = NoAction;
				this->_state.nextAction = NoAction;
				this->_state.nextOpponentAction = NoAction;
				this->logBattle(this->_state.opponentName + " wants to fight!");
				this->logBattle(this->_state.opponentName + " sent out " + this->_state.opponentTeam[0].getNickname() + "!");
				this->logBattle(this->_state.team[0].getName() + " go!");
				this->_log("Done: going to battle");
			}
			if (byte == SYNC_BYTE)
				return byte;
			break;
		case BATTLE:
			if (byte == 2) {
				this->_log("Back to PKMN Center");
				this->_stage = PKMN_CENTER;
				this->_timer = 0;
			} else if (byte == 0) {
				if (this->_state.nextOpponentAction == NoAction)
					break;
				this->_state.lastAction = this->_state.nextAction;
				this->_state.nextAction = NoAction;
				this->_state.nextOpponentAction = NoAction;
			} else if (byte != UNAVAILABLE_BYTE) {
				if (!this->_state.nextAction) {
					byte = UNAVAILABLE_BYTE;
					break;
				}
				if (!this->_state.nextOpponentAction) {
					this->_state.nextOpponentAction = static_cast<BattleAction>(byte);
					this->_executeBattleActions();
				}
				byte = this->_state.nextAction;
				handle.sendByte(0x00);
			} else if (this->_state.nextOpponentAction) {
				this->_state.lastAction = this->_state.nextAction;
				this->_state.nextAction = NoAction;
				this->_state.nextOpponentAction = NoAction;
				this->_syncSignalsReceived = 0;
			}
			break;
		}
		return byte;
	}

	void GameHandle::_makePlayersAttack(bool AIAttack, bool opponentAttack)
	{
		Pokemon &ai = this->_state.team[this->_state.pokemonOnField];
		Pokemon &opponent = this->_state.opponentTeam[this->_state.opponentPokemonOnField];
		int aiPriorityFactor = ai.getPriorityFactor(this->_state.nextAction - Attack1);
		int opponentPriorityFactor = opponent.getPriorityFactor(this->_state.nextOpponentAction - Attack1);
		bool aiStart = aiPriorityFactor > opponentPriorityFactor;

		if (aiPriorityFactor == opponentPriorityFactor)
			aiStart = (this->_randomGenerator() - 80 >= 0) ^ this->_isPlayer2;

		if (!ai.getHealth())
			return;
		if (AIAttack && aiStart)
			ai.attack(this->_state.nextAction - Attack1, opponent);
		if (!opponent.getHealth())
			return;
		if (opponentAttack)
			opponent.attack(this->_state.nextOpponentAction - Attack1, ai);
		if (!ai.getHealth())
			return;
		if (AIAttack && !aiStart)
			ai.attack(this->_state.nextAction - Attack1, opponent);
	}

	void GameHandle::setReady(bool ready)
	{
		this->_ready = ready;
	}

	void GameHandle::disconnect()
	{
		if (!this->isConnected())
			return;
		this->_emulator->disconnect();
	}

	void GameHandle::_executeBattleActions()
	{
		bool AIAttack = false;
		bool opponentAttack = false;
		bool AIFainted = !this->_state.team[this->_state.pokemonOnField].getHealth();
		bool opponentFainted = !this->_state.opponentTeam[this->_state.opponentPokemonOnField].getHealth();
		bool allyTeamOK = false;
		bool oppoTeamOK = false;

		for (const auto &pkmn : this->_state.team)
			if (pkmn.getHealth()) {
				allyTeamOK = true;
				break;
			}

		if (!allyTeamOK){
			this->logBattle(this->_trainerName + " is out of usable pokemon");
			this->logBattle(this->_trainerName + " blacked out!");
			this->_stage = PING_POKEMON_EXCHANGE;
			return;
		}

		for (const auto &pkmn : this->_state.opponentTeam)
			if (pkmn.getHealth()) {
				oppoTeamOK = true;
				break;
			}

		if (!oppoTeamOK){
			this->logBattle(this->_trainerName + " defeated " + this->_state.opponentName);
			this->_stage = PING_POKEMON_EXCHANGE;
			return;
		}

		if (this->_state.nextAction == NoAction && !this->_state.nextOpponentAction)
			this->_state.nextAction = this->_battleHandler(*this);

		if (AIFainted || !opponentFainted)
			switch (this->_state.nextAction) {
			case Run:
				this->logBattle("Got away safely!");
				this->_stage = PING_POKEMON_EXCHANGE;
				return;
			case Switch1:
			case Switch2:
			case Switch3:
			case Switch4:
			case Switch5:
			case Switch6:
				if (!AIFainted) {
					this->_state.team[this->_state.pokemonOnField].switched();
					this->logBattle(this->_state.team[this->_state.pokemonOnField].getName() + " come back!");
				}
				this->_state.pokemonOnField = this->_state.nextAction - Switch1;
				this->logBattle(this->_state.team[this->_state.pokemonOnField].getName() + " go!");
				break;
			case Attack1:
			case Attack2:
			case Attack3:
			case Attack4:
			case StruggleMove:
				if (!allyTeamOK)
					this->_log("Warning: Invalid AI move " + std::to_string(this->_state.nextAction));
				AIAttack = true;
				break;
			default:
				this->_log("Warning: Invalid AI move " + std::to_string(this->_state.nextAction));
				this->_state.nextAction = StruggleMove;
				AIAttack = true;
			}

		if (opponentFainted || !AIFainted)
			switch (this->_state.nextOpponentAction) {
			case Run:
				this->logBattle(this->_state.opponentTeam[this->_state.opponentPokemonOnField].getName() + " ran");
				this->_stage = PING_POKEMON_EXCHANGE;
				return;
			case Switch1:
			case Switch2:
			case Switch3:
			case Switch4:
			case Switch5:
			case Switch6:
				if (!opponentFainted) {
					this->_state.opponentTeam[this->_state.opponentPokemonOnField].switched();
					this->logBattle(this->_state.opponentName + " withdrew " + this->_state.opponentTeam[this->_state.opponentPokemonOnField].getNickname());
				}
				this->_state.opponentPokemonOnField = this->_state.nextOpponentAction - Switch1;
				this->logBattle(this->_state.opponentName + " sent out " + this->_state.opponentTeam[this->_state.opponentPokemonOnField].getNickname());
				break;
			case Attack1:
			case Attack2:
			case Attack3:
			case Attack4:
			case StruggleMove:
				if (!oppoTeamOK)
					this->_log("Warning: Invalid opponent move " + std::to_string(this->_state.nextAction));
				opponentAttack = true;
				break;
			default:
				this->_log("Warning: Invalid opponent move " + std::to_string(this->_state.nextOpponentAction));
				this->_state.nextOpponentAction = StruggleMove;
				opponentAttack = true;
			}

		if (opponentFainted || AIFainted)
			return;

		this->_makePlayersAttack(AIAttack, opponentAttack);
		this->_state.team[this->_state.pokemonOnField].endTurn();
		this->_state.opponentTeam[this->_state.opponentPokemonOnField].endTurn();
		this->_log(this->_state.team[this->_state.pokemonOnField].dump());
		this->_log(this->_state.opponentTeam[this->_state.opponentPokemonOnField].dump());

		allyTeamOK = false;
		oppoTeamOK = false;
		for (const auto &pkmn : this->_state.team)
			if (pkmn.getHealth()) {
				allyTeamOK = true;
				break;
			}

		if (!allyTeamOK){
			this->logBattle(this->_trainerName + " is out of usable pokemon");
			this->logBattle(this->_trainerName + " blacked out!");
			this->_stage = PING_POKEMON_EXCHANGE;
			return;
		}

		for (const auto &pkmn : this->_state.opponentTeam)
			if (pkmn.getHealth()) {
				oppoTeamOK = true;
				break;
			}

		if (!oppoTeamOK){
			this->logBattle(this->_trainerName + " defeated " + this->_state.opponentName);
			this->_stage = PING_POKEMON_EXCHANGE;
			return;
		}
	}

	Gen1ConnectionStage GameHandle::getStage() const
	{
		return this->_stage;
	}

	void GameHandle::_mainLoop(EmulatorHandle &handle)
	{
		static int val = 0;

		if (--val > 0 || this->_isPlayer2)
			return;
		switch (this->_stage) {
		case PINGING_OPPONENT:
			handle.sendByte(PING_BYTE);
			val = 500;
			break;
		case PING_POKEMON_EXCHANGE:
			if (!this->_ready)
				break;
			handle.sendByte(PING_BYTE);
			val = 1000;
			break;
		case ROOM_CHOOSE:
			if (this->_timer) {
				this->_buffer = 0;
				this->_timer--;
			} else if (!this->_done) {
				handle.sendByte(PING_BYTE);
			} else {
				handle.sendByte((209 + (this->_buffer / 3 % 2) * 4) * (this->_buffer % 3 != 2));
				this->_buffer++;
				if (this->_buffer >= 12)
					this->_buffer = 6;
			}
			val = 500;
			break;
		case EXCHANGE_POKEMONS:
			if (this->_timer == 0)
				this->_sent = false;
			else
				this->_timer--;
			if (this->_sent)
				break;
			this->_sent = true;
			this->_timer = 100;
			if (this->_sendBufferIndex.second < this->_sendBuffer[this->_sendBufferIndex.first].size()) {
				this->_syncSignalsSent = 0;
				handle.sendByte(this->_sendBuffer[this->_sendBufferIndex.first][this->_sendBufferIndex.second++]);
			} else if (this->_sendBufferIndex.first != 3) {
				handle.sendByte(SYNC_BYTE);
				this->_syncSignalsSent++;
			} else
				handle.sendByte(0x00);
			val = 200;
			break;
		case BATTLE:
			if (this->_state.nextOpponentAction) {
				handle.sendByte(0x00);
				val = 200;
			} else if (this->_state.nextAction == NoAction) {
				auto &pkmn = this->_state.team[this->_state.pokemonOnField];

				if (!pkmn.getHealth() || pkmn.getLastUsedMove().isFinished())
					this->_state.nextAction = this->_battleHandler(*this);
				else {
					this->_state.nextAction = this->_state.lastAction;
					this->_log("Forcing AI to keep using current move");
				}
				val = 100;
			} else {
				handle.sendByte(this->_state.nextAction);
				val = 400;
			}
			break;
		default:
			val = 0;
			break;
		}
	}

	void GameHandle::connect(const std::string &ip, unsigned short port)
	{
		if (this->isConnected())
			return;
		this->_emulator.reset(this->_emulatorMaker(this->_byteHandler, [this](EmulatorHandle &handle){ this->_mainLoop(handle); }, ip, port));
		this->_log("Connected to " + ip + ":" + std::to_string(port));
	}

	bool GameHandle::isConnected() const
	{
		return this->_emulator && this->_emulator->isConnected();
	}

	bool GameHandle::isReady() const
	{
		return this->_ready;
	}

	std::vector<unsigned char> GameHandle::convertString(const std::string &str)
	{
		std::vector<unsigned char> result;

		for (char c : str)
			result.push_back(ASCIIToPkmn1CharConversionTable[static_cast<unsigned char>(c)]);
		result.push_back(ASCIIToPkmn1CharConversionTable['\0']);
		return result;
	}

	std::string GameHandle::convertString(const std::vector<unsigned char> &str)
	{
		std::string result;

		for (unsigned char c : str) {
			c = Pkmn1CharToASCIIConversionTable[c];

			if (!c)
				break;

			try {
				result += _dispSpecialChars.at(c);
			} catch (std::out_of_range &) {
				result.push_back(c);
			}
		}
		return result;
	}

	void GameHandle::_interpretPacket()
	{
		this->_log("Decrypting received packet");
		if (this->_logMsg)
			displayPacket(this->_receiveBuffer);

		for (unsigned i = 0; i < this->_receiveBuffer.size(); i++) {
			if (this->_receiveBuffer[i] == UNAVAILABLE_BYTE) {
				this->_receiveBuffer.erase(this->_receiveBuffer.begin() + i);
				i--;
			}
		}

		/* HEADER PACKET */
		while (this->_receiveBuffer[0] == SYNC_BYTE)
			this->_receiveBuffer.erase(this->_receiveBuffer.begin());
		this->_receiveBuffer.erase(this->_receiveBuffer.begin(), this->_receiveBuffer.begin() + 10);

		/* Content */
		while (this->_receiveBuffer[0] == SYNC_BYTE)
			this->_receiveBuffer.erase(this->_receiveBuffer.begin());
		this->_state.opponentName = this->convertString(this->_receiveBuffer);
		this->_log("Playing against " + this->_state.opponentName);
		this->_receiveBuffer.erase(this->_receiveBuffer.begin(), this->_receiveBuffer.begin() + 11);

		unsigned char nbPkmns = this->_receiveBuffer[0];

		this->_log("They have " + std::to_string(nbPkmns) + " pokémon(s)");
		this->_receiveBuffer.erase(this->_receiveBuffer.begin());
		for (int i = 0; i < nbPkmns; i++)
			this->_log("Pokémon " + std::to_string(i) + ": " + pokemonList[this->_receiveBuffer[i]].name);
		this->_receiveBuffer.erase(this->_receiveBuffer.begin(), this->_receiveBuffer.begin() + 7);

		std::vector<unsigned char> pkmnData{this->_receiveBuffer.begin(), this->_receiveBuffer.begin() + 44 * 6};

		this->_receiveBuffer.erase(this->_receiveBuffer.begin(), this->_receiveBuffer.begin() + 44 * 6 + 66);
		this->_state.opponentTeam.clear();
		this->_state.opponentTeam.reserve(nbPkmns);
		for (int i = 0; i < nbPkmns; i++) {
			this->_state.opponentTeam.emplace_back(this->_randomGenerator, *this, this->convertString(this->_receiveBuffer), pkmnData, true);
			this->_receiveBuffer.erase(this->_receiveBuffer.begin(), this->_receiveBuffer.begin() + 11);
			pkmnData.erase(pkmnData.begin(), pkmnData.begin() + 44);
		}
		for (Pokemon &pkmn : this->_state.opponentTeam)
			this->_log(pkmn.dump());
	}

	void GameHandle::_log(const std::string &msg)
	{
		if (this->_logMsg || msg.substr(0, 7) == "Warning")
			(msg.substr(0, 7) == "Warning" ? std::cerr : std::cout) << "[Gen1GameHandle]: " << msg << std::endl;
	}

	std::vector<std::vector<unsigned char>> GameHandle::_craftPacket()
	{
		std::vector<std::vector<unsigned char>> packet;
		std::vector<unsigned char> buffer;

		this->_randomGenerator.makeRandomList(9);
		/*this->_randomGenerator.setList({
			0xC0, 0xED, 0x9E, 0x1D, 0x26, 0x56, 0x8F, 0xA7, 0x4A
		});*/
		packet.emplace_back();
		packet.push_back(this->_randomGenerator.getList());

		this->_log("Making packet for trainer " + this->_trainerName);
		buffer = this->convertString(this->_trainerName);
		buffer.resize(11, ASCIIToPkmn1CharConversionTable['\0']);
		for (unsigned i = 0; i < buffer.size(); i++)
			if (buffer[i] == UNAVAILABLE_BYTE)
				buffer[i] = 0xFF;

		if (this->_pkmns.empty()) {
			this->_log("Warning: Team is empty");
			this->addPokemonToTeam(
				"",
				1,
				PokemonGen1::pokemonList[PokemonGen1::Rhydon],
				std::vector<PokemonGen1::Move>{
					PokemonGen1::availableMoves[PokemonGen1::Tackle],
					PokemonGen1::availableMoves[PokemonGen1::Tail_Whip]
				}
			);
		}

		buffer.push_back(this->_pkmns.size());
		this->_log("Pushing " + std::to_string(this->_pkmns.size()) + " pokémon(s)");
		for (Pokemon &pkmn : this->_pkmns) {
			this->_log(pkmn.dump());
			buffer.push_back(pkmn.getID() != UNAVAILABLE_BYTE ? pkmn.getID() : 0xFF);
		}
		buffer.resize(19, 0xFF);

		for (auto &pkmn : this->_pkmns)
			for (unsigned char c : pkmn.encode())
				buffer.push_back(c != UNAVAILABLE_BYTE ? c : 0xFF);
		buffer.resize(19 + 44 * 6, 0);

		packet.push_back(buffer);
		buffer = this->convertString(this->_trainerName);
		buffer.resize(11, ASCIIToPkmn1CharConversionTable['\0']);
		for (unsigned char c : buffer)
			packet[2].push_back(c != UNAVAILABLE_BYTE ? c : 0xFF);
		packet[2].resize(packet[2].size() + 55, 0);

		for (Pokemon &pkmn : this->_pkmns) {
			buffer = this->convertString(pkmn.getName());
			buffer.resize(11, '\x50');
			buffer[10] = ASCIIToPkmn1CharConversionTable['\0'];
			for (unsigned char c : buffer)
				packet[2].push_back(c != UNAVAILABLE_BYTE ? c : 0xFF);
		}
		packet[2].resize(415, ASCIIToPkmn1CharConversionTable['\0']);
		packet[2].resize(packet[2].size() + 21, 0);
		packet[2].resize(packet[2].size() + 3, 0xFF);
		packet.push_back(PACKET_FOOTER);

		this->_log("Packet ready");
		if (this->_logMsg)
			for (auto &p : packet)
				displayPacket(p);
		return packet;
	}

	const std::string &GameHandle::getTrainerName() const
	{
		return this->_trainerName;
	}

	void GameHandle::setTrainerName(const std::string &trainerName)
	{
		this->_trainerName = trainerName;
	}

	void GameHandle::deletePokemon(unsigned char index)
	{
		if (index >= this->_pkmns.size() || this->_pkmns.size() <= 1)
			return;

		std::vector<Pokemon> buffer;

		if (this->_pkmns.size() <= index)
			throw std::out_of_range("Out of range");

		for (size_t i = 0; i < this->_pkmns.size(); i++) {
			if (i != index)
				buffer.push_back(this->_pkmns[i]);
		}

		this->_pkmns.clear();
		for (const Pokemon &pkmn : buffer)
			this->_pkmns.push_back(pkmn);
	}

	Pokemon &GameHandle::getPokemon(unsigned int index)
	{
		return this->_pkmns.at(index);
	}

	std::pair<unsigned, unsigned> GameHandle::getBattleSendingProgress()
	{
		if (this->getStage() != EXCHANGE_POKEMONS)
			return {0, 0};

		std::pair<unsigned, unsigned> progress;

		for (unsigned i = 0; i < this->_sendBufferIndex.first; i++)
			progress.first += this->_sendBuffer[i].size();
		progress.first += this->_sendBufferIndex.second;
		for (auto &elem : this->_sendBuffer)
			progress.second += elem.size();
		return progress;
	}

	std::vector<unsigned char> GameHandle::save()
	{
		std::vector<unsigned char> data = this->convertString(this->_trainerName);

		data.reserve(11 * 7 + 44 * 6 + 1);

		data.resize(10, ASCIIToPkmn1CharConversionTable['\0']);
		data.push_back(ASCIIToPkmn1CharConversionTable['\0']);
		data.push_back(this->_pkmns.size());

		for (auto &pkmn : this->_pkmns) {
			auto encoded = pkmn.encode();

			data.insert(data.end(), encoded.begin(), encoded.end());
		}
		data.resize(11 + 44 * 6 + 1, 0);

		for (Pokemon &pkmn : this->_pkmns) {
			auto buf = this->convertString(pkmn.getNickname());

			buf.resize(10, ASCIIToPkmn1CharConversionTable['\0']);
			buf.push_back(ASCIIToPkmn1CharConversionTable['\0']);
			data.insert(data.end(), buf.begin(), buf.end());
		}
		data.resize(11 * 7 + 44 * 6 + 1);

		return data;
	}

	void GameHandle::load(const std::vector<unsigned char> &d)
	{
		std::vector<unsigned char> data = d;

		if (d.size() != 11 * 7 + 44 * 6 + 1)
			throw InvalidSaveFileException("The data size doesn't match (expected 342B but got " + std::to_string(d.size()) + "B)");

		/* Content */
		this->_trainerName = this->convertString(data);
		if (this->_trainerName.size() > 10)
			throw InvalidSaveFileException("The trainer name \"" + this->_trainerName + "\" is too long (Expected at most 10 characters but got " + std::to_string(this->_trainerName.size()) + ")");
		data.erase(data.begin(), data.begin() + 11);

		unsigned char nbPkmns = data[0];

		if (nbPkmns > 6 || !nbPkmns)
			throw InvalidSaveFileException("The number of pokémons is invalid (Expected between 1 and 6 pokémons but got " + std::to_string(nbPkmns) + ")");
		data.erase(data.begin());

		std::vector<unsigned char> pkmnData{data.begin(), data.begin() + 44 * 6};

		data.erase(data.begin(), data.begin() + 44 * 6);
		this->_pkmns.clear();
		this->_pkmns.reserve(nbPkmns);
		for (int i = 0; i < nbPkmns; i++) {
			this->_pkmns.emplace_back(this->_randomGenerator, *this, this->convertString(data), pkmnData, false);
			data.erase(data.begin(), data.begin() + 11);
			pkmnData.erase(pkmnData.begin(), pkmnData.begin() + 44);
		}
	}

	/*
	** From Rhydon
	** https://github.com/SciresM/Rhydon/blob/2056e8f044d3c5178ad2d697d0823d2b799bb099/Rhydon/Encoding.cs#L122
	*/
	const std::vector<unsigned char> ASCIIToPkmn1CharConversionTable{
		0x50, /* 0x00 */
		0x7F, /* 0x01 */
		0x7F, /* 0x02 */
		0x7F, /* 0x03 */
		0x7F, /* 0x04 */
		0x7F, /* 0x05 */
		0x7F, /* 0x06 */
		0x7F, /* 0x07 */
		0x7F, /* 0x08 */
		0x7F, /* 0x09 */
		0x7F, /* 0x0A */
		0x7F, /* 0x0B */
		0x7F, /* 0x0C */
		0x7F, /* 0x0D */
		0x7F, /* 0x0E */
		0x7F, /* 0x0F */
		0x7F, /* 0x10 */
		0x7F, /* 0x11 */
		0x7F, /* 0x12 */
		0x7F, /* 0x13 */
		0x7F, /* 0x14 */
		0x7F, /* 0x15 */
		0x7F, /* 0x16 */
		0x7F, /* 0x17 */
		0x7F, /* 0x18 */
		0x7F, /* 0x19 */
		0x7F, /* 0x1A */
		0x7F, /* 0x1B */
		0x7F, /* 0x1C */
		0x7F, /* 0x1D */
		0x7F, /* 0x1E */
		0x7F, /* 0x1F */
		0x7F, /* 0x20 ' ' */
		0xE7, /* 0x21 '!' */
		0x7F, /* 0x22 '"' */
		0x7F, /* 0x23 '#' */
		0x7F, /* 0x24 '$' */
		0x7F, /* 0x25 '%' */
		0x7F, /* 0x26 '&' */
		0x7F, /* 0x27 '\''*/
		0x9A, /* 0x28 '(' */
		0x9B, /* 0x29 ')' */
		0x7F, /* 0x2A '*' */
		0x7F, /* 0x2B '+' */
		0xF4, /* 0x2C ',' */
		0xE3, /* 0x2D '-' */
		0xF2, /* 0x2E '.' */
		0xF3, /* 0x2F '/' */
		0xF6, /* 0x30 '0' */
		0xF7, /* 0x31 '1' */
		0xF8, /* 0x32 '2' */
		0xF9, /* 0x33 '3' */
		0xFA, /* 0x34 '4' */
		0xFB, /* 0x35 '5' */
		0xFC, /* 0x36 '6' */
		0xFD, /* 0x37 '7' */
		0xFE, /* 0x38 '8' */
		0xFF, /* 0x39 '9' */
		0x9C, /* 0x3A ':' */
		0x9D, /* 0x3B ';' */
		0x7F, /* 0x3C '<' */
		0x7F, /* 0x3D '=' */
		0x7F, /* 0x3E '>' */
		0xE6, /* 0x3F '?' */
		0x7F, /* 0x40 '@' */
		0x80, /* 0x41 'A' */
		0x81, /* 0x42 'B' */
		0x82, /* 0x43 'C' */
		0x83, /* 0x44 'D' */
		0x84, /* 0x45 'E' */
		0x85, /* 0x46 'F' */
		0x86, /* 0x47 'G' */
		0x87, /* 0x48 'H' */
		0x88, /* 0x49 'I' */
		0x89, /* 0x4A 'J' */
		0x8A, /* 0x4B 'K' */
		0x8B, /* 0x4C 'L' */
		0x8C, /* 0x4D 'M' */
		0x8D, /* 0x4E 'N' */
		0x8E, /* 0x4F 'O' */
		0x8F, /* 0x50 'P' */
		0x90, /* 0x51 'Q' */
		0x91, /* 0x52 'R' */
		0x92, /* 0x53 'S' */
		0x93, /* 0x54 'T' */
		0x94, /* 0x55 'U' */
		0x95, /* 0x56 'V' */
		0x96, /* 0x57 'W' */
		0x97, /* 0x58 'X' */
		0x98, /* 0x59 'Y' */
		0x99, /* 0x5A 'Z' */
		0x9E, /* 0x5B '[' */
		0x7F, /* 0x5C '\\'*/
		0x9F, /* 0x5D ']' */
		0x7F, /* 0x5E '^' */
		0x7F, /* 0x5F '_' */
		0xF5, /* 0x60 '`' */
		0xA0, /* 0x61 'a' */
		0xA1, /* 0x62 'b' */
		0xA2, /* 0x63 'c' */
		0xA3, /* 0x64 'd' */
		0xA4, /* 0x65 'e' */
		0xA5, /* 0x66 'f' */
		0xA6, /* 0x67 'g' */
		0xA7, /* 0x68 'h' */
		0xA8, /* 0x69 'i' */
		0xA9, /* 0x6A 'j' */
		0xAA, /* 0x6B 'k' */
		0xAB, /* 0x6C 'l' */
		0xAC, /* 0x6D 'm' */
		0xAD, /* 0x6E 'n' */
		0xAE, /* 0x6F 'o' */
		0xAF, /* 0x70 'p' */
		0xB0, /* 0x71 'q' */
		0xB1, /* 0x72 'r' */
		0xB2, /* 0x73 's' */
		0xB3, /* 0x74 't' */
		0xB4, /* 0x75 'u' */
		0xB5, /* 0x76 'v' */
		0xB6, /* 0x77 'w' */
		0xB7, /* 0x78 'x' */
		0xB8, /* 0x79 'y' */
		0xB9, /* 0x7A 'z' */
		0xE1, /* 0x7B '{' */
		0x7F, /* 0x7C '|' */
		0xE2, /* 0x7D '}' */
		0xEF, /* 0x7E '~' */
		0x7F, /* 0x7F */
		0x7F, /* 0x80 */
		0x7F, /* 0x81 */
		0x7F, /* 0x82 */
		0x7F, /* 0x83 */
		0x7F, /* 0x84 */
		0x7F, /* 0x85 */
		0x7F, /* 0x86 */
		0x7F, /* 0x87 */
		0x7F, /* 0x88 */
		0x7F, /* 0x89 */
		0x7F, /* 0x8A */
		0x7F, /* 0x8B */
		0x7F, /* 0x8C */
		0x7F, /* 0x8D */
		0x7F, /* 0x8E */
		0x7F, /* 0x8F */
		0x7F, /* 0x90 */
		0x7F, /* 0x91 */
		0x7F, /* 0x92 */
		0x7F, /* 0x93 */
		0x7F, /* 0x94 */
		0x7F, /* 0x95 */
		0x7F, /* 0x96 */
		0x7F, /* 0x97 */
		0x7F, /* 0x98 */
		0x7F, /* 0x99 */
		0x7F, /* 0x9A */
		0x7F, /* 0x9B */
		0x7F, /* 0x9C */
		0x7F, /* 0x9D */
		0x7F, /* 0x9E */
		0x7F, /* 0x9F */
		0x7F, /* 0xA0 */
		0x7F, /* 0xA1 */
		0x7F, /* 0xA2 */
		0x7F, /* 0xA3 */
		0x7F, /* 0xA4 */
		0x7F, /* 0xA5 */
		0x7F, /* 0xA6 */
		0x7F, /* 0xA7 */
		0x7F, /* 0xA8 */
		0x7F, /* 0xA9 */
		0x7F, /* 0xAA */
		0x7F, /* 0xAB */
		0x7F, /* 0xAC */
		0x7F, /* 0xAD */
		0x7F, /* 0xAE */
		0x7F, /* 0xAF */
		0x7F, /* 0xB0 */
		0x7F, /* 0xB1 */
		0x7F, /* 0xB2 */
		0x7F, /* 0xB3 */
		0x7F, /* 0xB4 */
		0x7F, /* 0xB5 */
		0x7F, /* 0xB6 */
		0x7F, /* 0xB7 */
		0x7F, /* 0xB8 */
		0x7F, /* 0xB9 */
		0x7F, /* 0xBA */
		0x7F, /* 0xBB */
		0x7F, /* 0xBC */
		0x7F, /* 0xBD */
		0x7F, /* 0xBE */
		0x7F, /* 0xBF */
		0x7F, /* 0xC0 */
		0x7F, /* 0xC1 */
		0x7F, /* 0xC2 */
		0x7F, /* 0xC3 */
		0x7F, /* 0xC4 */
		0x7F, /* 0xC5 */
		0x7F, /* 0xC6 */
		0x7F, /* 0xC7 */
		0x7F, /* 0xC8 */
		0x7F, /* 0xC9 */
		0x7F, /* 0xCA */
		0x7F, /* 0xCB */
		0x7F, /* 0xCC */
		0x7F, /* 0xCD */
		0x7F, /* 0xCE */
		0x7F, /* 0xCF */
		0x7F, /* 0xD0 */
		0x7F, /* 0xD1 */
		0x7F, /* 0xD2 */
		0x7F, /* 0xD3 */
		0x7F, /* 0xD4 */
		0x7F, /* 0xD5 */
		0x7F, /* 0xD6 */
		0x7F, /* 0xD7 */
		0x7F, /* 0xD8 */
		0x7F, /* 0xD9 */
		0x7F, /* 0xDA */
		0x7F, /* 0xDB */
		0x7F, /* 0xDC */
		0x7F, /* 0xDD */
		0x7F, /* 0xDE */
		0x7F, /* 0xDF */
		0x7F, /* 0xE0 */
		0x7F, /* 0xE1 */
		0x7F, /* 0xE2 */
		0x7F, /* 0xE3 */
		0x7F, /* 0xE4 */
		0x7F, /* 0xE5 */
		0x7F, /* 0xE6 */
		0x7F, /* 0xE7 */
		0x7F, /* 0xE8 */
		0x7F, /* 0xE9 */
		0x7F, /* 0xEA */
		0x7F, /* 0xEB */
		0x7F, /* 0xEC */
		0x7F, /* 0xED */
		0x7F, /* 0xEE */
		0x7F, /* 0xEF CHAR_ARR_DOW */
		0x7F, /* 0xF0 CHAR_ARR_BLA */
		0x7F, /* 0xF1 CHAR_ARR_WHI */
		0x7F, /* 0xF2 CHAR_AP_M    */
		0x7F, /* 0xF3 CHAR_AP_R    */
		0x7F, /* 0xF4 CHAR_AP_V    */
		0x7F, /* 0xF5 CHAR_AP_T    */
		0x7F, /* 0xF6 CHAR_AP_S    */
		0x7F, /* 0xF7 CHAR_AP_L    */
		0x7F, /* 0xF8 CHAR_AP_D    */
		0x7F, /* 0xF9 CHAR_E_ACCENT*/
		0x7F, /* 0xFA CHAR_FEM_NUM */
		0x7F, /* 0xFB CHAR_MAL_NUM */
		0x7F, /* 0xFC CHAR_MN_NUM  */
		0x7F, /* 0xFD CHAR_PK_NUM  */
		0x5D, /* 0xFE CHAR_TRAINER */
		0x7F  /* 0xFF */
	};

	/*
	** From Rhydon
	** https://github.com/SciresM/Rhydon/blob/2056e8f044d3c5178ad2d697d0823d2b799bb099/Rhydon/Encoding.cs#L381
	** Used BGB emulator debugger to add more characters
	*/
	const std::vector<unsigned char> Pkmn1CharToASCIIConversionTable{
		CHAR_INVALID, /* 0x00 */
		CHAR_INVALID, /* 0x01 */
		CHAR_INVALID, /* 0x02 */
		CHAR_INVALID, /* 0x03 */
		CHAR_INVALID, /* 0x04 */
		CHAR_INVALID, /* 0x05 */
		CHAR_INVALID, /* 0x06 */
		CHAR_INVALID, /* 0x07 */
		CHAR_INVALID, /* 0x08 */
		CHAR_INVALID, /* 0x09 */
		CHAR_INVALID, /* 0x0A */
		CHAR_INVALID, /* 0x0B */
		CHAR_INVALID, /* 0x0C */
		CHAR_INVALID, /* 0x0D */
		CHAR_INVALID, /* 0x0E */
		CHAR_INVALID, /* 0x0F */
		CHAR_INVALID, /* 0x10 */
		CHAR_INVALID, /* 0x11 */
		CHAR_INVALID, /* 0x12 */
		CHAR_INVALID, /* 0x13 */
		CHAR_INVALID, /* 0x14 */
		CHAR_INVALID, /* 0x15 */
		CHAR_INVALID, /* 0x16 */
		CHAR_INVALID, /* 0x17 */
		CHAR_INVALID, /* 0x18 */
		CHAR_INVALID, /* 0x19 */
		CHAR_INVALID, /* 0x1A */
		CHAR_INVALID, /* 0x1B */
		CHAR_INVALID, /* 0x1C */
		CHAR_INVALID, /* 0x1D */
		CHAR_INVALID, /* 0x1E */
		CHAR_INVALID, /* 0x1F */
		CHAR_INVALID, /* 0x20 */
		CHAR_INVALID, /* 0x21 */
		CHAR_INVALID, /* 0x22 */
		CHAR_INVALID, /* 0x23 */
		CHAR_INVALID, /* 0x24 */
		CHAR_INVALID, /* 0x25 */
		CHAR_INVALID, /* 0x26 */
		CHAR_INVALID, /* 0x27 */
		CHAR_INVALID, /* 0x28 */
		CHAR_INVALID, /* 0x29 */
		CHAR_INVALID, /* 0x2A */
		CHAR_INVALID, /* 0x2B */
		CHAR_INVALID, /* 0x2C */
		CHAR_INVALID, /* 0x2D */
		CHAR_INVALID, /* 0x2E */
		CHAR_INVALID, /* 0x2F */
		CHAR_INVALID, /* 0x30 */
		CHAR_INVALID, /* 0x31 */
		CHAR_INVALID, /* 0x32 */
		CHAR_INVALID, /* 0x33 */
		CHAR_INVALID, /* 0x34 */
		CHAR_INVALID, /* 0x35 */
		CHAR_INVALID, /* 0x36 */
		CHAR_INVALID, /* 0x37 */
		CHAR_INVALID, /* 0x38 */
		CHAR_INVALID, /* 0x39 */
		CHAR_INVALID, /* 0x3A */
		CHAR_INVALID, /* 0x3B */
		CHAR_INVALID, /* 0x3C */
		CHAR_INVALID, /* 0x3D */
		CHAR_INVALID, /* 0x3E */
		CHAR_INVALID, /* 0x3F */
		CHAR_INVALID, /* 0x40 */
		CHAR_INVALID, /* 0x41 */
		CHAR_INVALID, /* 0x42 */
		CHAR_INVALID, /* 0x43 */
		CHAR_INVALID, /* 0x44 */
		CHAR_INVALID, /* 0x45 */
		CHAR_INVALID, /* 0x46 */
		CHAR_INVALID, /* 0x47 */
		CHAR_INVALID, /* 0x48 */
		CHAR_INVALID, /* 0x49 */
		CHAR_INVALID, /* 0x4A */
		CHAR_INVALID, /* 0x4B */
		CHAR_INVALID, /* 0x4C */
		CHAR_INVALID, /* 0x4D */
		CHAR_INVALID, /* 0x4E */
		CHAR_INVALID, /* 0x4F */
		'\0',         /* 0x50 */
		CHAR_INVALID, /* 0x51 */
		CHAR_INVALID, /* 0x52 */
		CHAR_INVALID, /* 0x53 */
		CHAR_INVALID, /* 0x54 */
		CHAR_INVALID, /* 0x55 */
		CHAR_INVALID, /* 0x56 */
		CHAR_INVALID, /* 0x57 */
		CHAR_INVALID, /* 0x58 */
		CHAR_INVALID, /* 0x59 */
		CHAR_INVALID, /* 0x5A */
		CHAR_INVALID, /* 0x5B */
		CHAR_INVALID, /* 0x5C */
		CHAR_TRAINER, /* 0x5D */
		CHAR_INVALID, /* 0x5E */
		CHAR_INVALID, /* 0x5F */
		CHAR_INVALID, /* 0x60 */
		CHAR_INVALID, /* 0x61 */
		CHAR_INVALID, /* 0x62 */
		CHAR_INVALID, /* 0x63 */
		CHAR_INVALID, /* 0x64 */
		CHAR_INVALID, /* 0x65 */
		CHAR_INVALID, /* 0x66 */
		CHAR_INVALID, /* 0x67 */
		CHAR_INVALID, /* 0x68 */
		CHAR_INVALID, /* 0x69 */
		CHAR_INVALID, /* 0x6A */
		CHAR_INVALID, /* 0x6B */
		CHAR_INVALID, /* 0x6C */
		CHAR_INVALID, /* 0x6D */
		CHAR_INVALID, /* 0x6E */
		CHAR_INVALID, /* 0x6F */
		CHAR_INVALID, /* 0x70 */
		CHAR_INVALID, /* 0x71 */
		CHAR_INVALID, /* 0x72 */
		CHAR_INVALID, /* 0x73 */
		CHAR_INVALID, /* 0x74 */
		CHAR_INVALID, /* 0x75 */
		CHAR_INVALID, /* 0x76 */
		CHAR_INVALID, /* 0x77 */
		CHAR_INVALID, /* 0x78 */
		CHAR_INVALID, /* 0x79 */
		CHAR_INVALID, /* 0x7A */
		CHAR_INVALID, /* 0x7B */
		CHAR_INVALID, /* 0x7C */
		CHAR_INVALID, /* 0x7D */
		CHAR_INVALID, /* 0x7E */
		' ',          /* 0x7F */
		'A',          /* 0x80 */
		'B',          /* 0x81 */
		'C',          /* 0x82 */
		'D',          /* 0x83 */
		'E',          /* 0x84 */
		'F',          /* 0x85 */
		'G',          /* 0x86 */
		'H',          /* 0x87 */
		'I',          /* 0x88 */
		'J',          /* 0x89 */
		'K',          /* 0x8A */
		'L',          /* 0x8B */
		'M',          /* 0x8C */
		'N',          /* 0x8D */
		'O',          /* 0x8E */
		'P',          /* 0x8F */
		'Q',          /* 0x90 */
		'R',          /* 0x91 */
		'S',          /* 0x92 */
		'T',          /* 0x93 */
		'U',          /* 0x94 */
		'V',          /* 0x95 */
		'W',          /* 0x96 */
		'X',          /* 0x97 */
		'Y',          /* 0x98 */
		'Z',          /* 0x99 */
		'(',          /* 0x9A */
		')',          /* 0x9B */
		':',          /* 0x9C */
		';',          /* 0x9D */
		'[',          /* 0x9E */
		']',          /* 0x9F */
		'a',          /* 0xA0 */
		'b',          /* 0xA1 */
		'c',          /* 0xA2 */
		'd',          /* 0xA3 */
		'e',          /* 0xA4 */
		'f',          /* 0xA5 */
		'g',          /* 0xA6 */
		'h',          /* 0xA7 */
		'i',          /* 0xA8 */
		'j',          /* 0xA9 */
		'k',          /* 0xAA */
		'l',          /* 0xAB */
		'm',          /* 0xAC */
		'n',          /* 0xAD */
		'o',          /* 0xAE */
		'p',          /* 0xAF */
		'q',          /* 0xB0 */
		'r',          /* 0xB1 */
		's',          /* 0xB2 */
		't',          /* 0xB3 */
		'u',          /* 0xB4 */
		'v',          /* 0xB5 */
		'w',          /* 0xB6 */
		'x',          /* 0xB7 */
		'y',          /* 0xB8 */
		'z',          /* 0xB9 */
		CHAR_E_ACCENT,/* 0xBA */
		CHAR_AP_D,    /* 0xBB */
		CHAR_AP_L,    /* 0xBC */
		CHAR_AP_S,    /* 0xBD */
		CHAR_AP_T,    /* 0xBE */
		CHAR_AP_V,    /* 0xBF */
		CHAR_INVALID, /* 0xC0 */
		CHAR_INVALID, /* 0xC1 */
		CHAR_INVALID, /* 0xC2 */
		CHAR_INVALID, /* 0xC3 */
		CHAR_INVALID, /* 0xC4 */
		CHAR_INVALID, /* 0xC5 */
		CHAR_INVALID, /* 0xC6 */
		CHAR_INVALID, /* 0xC7 */
		CHAR_INVALID, /* 0xC8 */
		CHAR_INVALID, /* 0xC9 */
		CHAR_INVALID, /* 0xCA */
		CHAR_INVALID, /* 0xCB */
		CHAR_INVALID, /* 0xCC */
		CHAR_INVALID, /* 0xCD */
		CHAR_INVALID, /* 0xCE */
		CHAR_INVALID, /* 0xCF */
		CHAR_INVALID, /* 0xD0 */
		CHAR_INVALID, /* 0xD1 */
		CHAR_INVALID, /* 0xD2 */
		CHAR_INVALID, /* 0xD3 */
		CHAR_INVALID, /* 0xD4 */
		CHAR_INVALID, /* 0xD5 */
		CHAR_INVALID, /* 0xD6 */
		CHAR_INVALID, /* 0xD7 */
		CHAR_INVALID, /* 0xD8 */
		CHAR_INVALID, /* 0xD9 */
		CHAR_INVALID, /* 0xDA */
		CHAR_INVALID, /* 0xDB */
		CHAR_INVALID, /* 0xDC */
		CHAR_INVALID, /* 0xDD */
		CHAR_INVALID, /* 0xDE */
		CHAR_INVALID, /* 0xDF */
		'\'',         /* 0xE0 */
		CHAR_PK_NUM,  /* 0xE1 */
		CHAR_MN_NUM,  /* 0xE2 */
		'-',          /* 0xE3 */
		CHAR_AP_R,    /* 0xE4 */
		CHAR_AP_M,    /* 0xE5 */
		'?',          /* 0xE6 */
		'!',          /* 0xE7 */
		'.',          /* 0xE8 */
		CHAR_INVALID, /* 0xE9 */
		CHAR_INVALID, /* 0xEA */
		CHAR_INVALID, /* 0xEB */
		CHAR_ARR_WHI, /* 0xEC */
		CHAR_ARR_BLA, /* 0xED */
		CHAR_ARR_DOW, /* 0xEE */
		CHAR_MAL_NUM, /* 0xEF */
		'$',          /* 0xF0 */
		'x',          /* 0xF1 */
		'.',          /* 0xF2 */
		'/',          /* 0xF3 */
		',',          /* 0xF4 */
		CHAR_FEM_NUM, /* 0xF5 */
		'0',          /* 0xF6 */
		'1',          /* 0xF7 */
		'2',          /* 0xF8 */
		'3',          /* 0xF9 */
		'4',          /* 0xFA */
		'5',          /* 0xFB */
		'6',          /* 0xFC */
		'7',          /* 0xFD */
		'8',          /* 0xFE */
		'9'           /* 0xFF */
	};
}
