//
// Created by PinkySmile on 19/06/25.
//

#include <thread>
#include "EmulatorGameHandle.hpp"
#include "../Exception.hpp"

namespace PokemonGen1
{
	const std::map<char, const char *> EmulatorGameHandle::_dispSpecialChars{
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

	void EmulatorGameHandle::_displayPacket(std::vector<unsigned char> packet)
	{
		for (unsigned int i = 0; i < packet.size(); i += 20) {
			for (unsigned j = 0; j < 20 && j + i < packet.size(); j++)
				printf("%02X ", packet[j + i]);
			for (int j = 0; j < static_cast<int>(i - packet.size() + 20); j++)
				printf("   ");
			for (unsigned j = 0; j < 20 && j + i < packet.size(); j++) {
				auto c = packet[j + i];

				printf("%c", isprint(c) ? c : '.');
			}
			for (int j = 0; j < static_cast<int>(i - packet.size() + 20); j++)
				printf(" ");
			printf(" ");
			for (unsigned j = 0; j < 20 && j + i < packet.size(); j++) {
				auto c = Pkmn1CharToASCIIConversionTable[packet[j + i]];

				printf("%c", isprint(c) ? c : '.');
			}
			printf("\n");
		}
	}

	EmulatorGameHandle::EmulatorGameHandle(
		const EmulatorCreator &emulatorMaker,
		BattleState &state,
		bool player2,
		bool log
	) :
		_logMsg(log),
		_isPlayer2(player2),
		_state(state),
		_emulator(emulatorMaker(this->_byteHandler, this->_loopHandle))
	{
	}

	unsigned char EmulatorGameHandle::_handleReceivedBytes(EmulatorHandle &handle, unsigned char byte)
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
				this->_sendBuffer = this->_craftPacket(this->_state);
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
				this->_state.me.pokemonOnField = 0;
				this->_state.op.pokemonOnField = 0;
				this->_state.me.lastAction = NoAction;
				this->_state.op.lastAction = NoAction;
				this->_state.me.nextAction = NoAction;
				this->_state.op.nextAction = NoAction;
				this->_log("Done: going to battle");
				if (this->_state.onBattleStart)
					this->_state.onBattleStart();
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
				if (this->_state.op.nextAction == NoAction)
					break;
				this->_state.me.lastAction = this->_state.me.nextAction;
				this->_state.op.lastAction = this->_state.op.nextAction;
				this->_state.me.nextAction = NoAction;
				this->_state.op.nextAction = NoAction;
			} else if (byte != UNAVAILABLE_BYTE) {
				if (this->_state.me.nextAction == NoAction) {
					byte = UNAVAILABLE_BYTE;
					break;
				}
				if (this->_state.op.nextAction == NoAction)
					this->_state.op.nextAction = static_cast<BattleAction>(byte);
				byte = this->_state.me.nextAction;
				handle.sendByte(0x00);
			} else if (this->_state.op.nextAction) {
				if (this->_state.onTurnStart && this->_state.onTurnStart()) {
					this->_stage = PING_POKEMON_EXCHANGE;
					this->_log("Game ended");
				}
				this->_state.me.lastAction = this->_state.me.nextAction;
				this->_state.op.lastAction = this->_state.op.nextAction;
				this->_state.me.nextAction = NoAction;
				this->_state.op.nextAction = NoAction;
				this->_syncSignalsReceived = 0;
			}
			break;
		}
		return byte;
	}

	void EmulatorGameHandle::setReady(bool ready)
	{
		this->_ready = ready;
	}

	EmulatorGameHandle::Gen1ConnectionStage EmulatorGameHandle::getStage() const
	{
		return this->_stage;
	}

	void EmulatorGameHandle::setStage(Gen1ConnectionStage stage)
	{
		this->_stage = stage;
	}

	void EmulatorGameHandle::_mainLoop(EmulatorHandle &handle)
	{
		if (this->_isPlayer2)
			return;
		if (this->_val > 0) {
			this->_val--;
			return;
		}
		switch (this->_stage) {
		case PINGING_OPPONENT:
			handle.sendByte(PING_BYTE);
			this->_val = 500;
			break;
		case PING_POKEMON_EXCHANGE:
			if (!this->_ready)
				break;
			handle.sendByte(PING_BYTE);
			this->_val = 1000;
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
			this->_val = 500;
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
			this->_val = 200;
			break;
		case BATTLE:
			if (this->_state.op.nextAction != NoAction) {
				handle.sendByte(0x00);
				this->_val = 200;
			} else if (this->_state.me.nextAction == NoAction) {
				this->_val = 100;
			} else {
				handle.sendByte(this->_state.me.nextAction);
				this->_val = 400;
			}
			break;
		default:
			this->_val = 0;
			break;
		}
	}

	bool EmulatorGameHandle::isConnected() const
	{
		return this->_emulator && this->_emulator->isConnected();
	}

	bool EmulatorGameHandle::isReady() const
	{
		return this->_ready;
	}

	std::vector<unsigned char> EmulatorGameHandle::convertString(const std::string &str)
	{
		std::vector<unsigned char> result;

		for (char c : str)
			result.push_back(ASCIIToPkmn1CharConversionTable[static_cast<unsigned char>(c)]);
		result.push_back(ASCIIToPkmn1CharConversionTable['\0']);
		return result;
	}

	std::string EmulatorGameHandle::convertString(const std::vector<unsigned char> &str)
	{
		std::string result;

		for (unsigned char c : str) {
			c = Pkmn1CharToASCIIConversionTable[c];

			if (!c)
				break;

			try {
				result += EmulatorGameHandle::_dispSpecialChars.at(c);
			} catch (std::out_of_range &) {
				result.push_back(c);
			}
		}
		return result;
	}

	void EmulatorGameHandle::_interpretPacket()
	{
		this->_log("Decrypting received packet");
		if (this->_logMsg)
			EmulatorGameHandle::_displayPacket(this->_receiveBuffer);

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
		this->_state.op.name = this->convertString(this->_receiveBuffer);
		this->_log("Playing against " + this->_state.op.name);
		this->_receiveBuffer.erase(this->_receiveBuffer.begin(), this->_receiveBuffer.begin() + 11);

		unsigned char nbPkmns = this->_receiveBuffer[0];

		this->_log("They have " + std::to_string(nbPkmns) + " pokémon(s)");
		this->_receiveBuffer.erase(this->_receiveBuffer.begin());
		for (int i = 0; i < nbPkmns; i++)
			this->_log("Pokémon " + std::to_string(i) + ": " + pokemonList.at(this->_receiveBuffer[i]).name);
		this->_receiveBuffer.erase(this->_receiveBuffer.begin(), this->_receiveBuffer.begin() + 7);

		std::array<std::array<unsigned char, Pokemon::ENCODED_SIZE>, 6> pkmnData;
		auto it = this->_receiveBuffer.begin();

		for (size_t i = 0; i < 6; i++) {
			std::copy(it, it + Pokemon::ENCODED_SIZE, pkmnData[i].begin());
			it += Pokemon::ENCODED_SIZE;
		}

		this->_receiveBuffer.erase(this->_receiveBuffer.begin(), this->_receiveBuffer.begin() + Pokemon::ENCODED_SIZE * 6 + 66);
		this->_state.op.team.clear();
		this->_state.op.team.reserve(nbPkmns);
		for (int i = 0; i < nbPkmns; i++) {
			this->_state.op.team.emplace_back(this->_state.rng, this->_state.battleLogger, this->convertString(this->_receiveBuffer), pkmnData[i], true);
			this->_receiveBuffer.erase(this->_receiveBuffer.begin(), this->_receiveBuffer.begin() + 11);
		}
		for (const Pokemon &pkmn : this->_state.op.team)
			this->_log(pkmn.dump());
	}

	void EmulatorGameHandle::_log(const std::string &msg) const
	{
		if (this->_logMsg || msg.substr(0, 7) == "Warning")
			(msg.substr(0, 7) == "Warning" ? std::cerr : std::cout) << "[Gen1EmulatorGameHandle]: " << msg << std::endl;
	}

	std::vector<std::vector<unsigned char>> EmulatorGameHandle::_craftPacket(const BattleState &state)
	{
		std::vector<std::vector<unsigned char>> packet;
		std::vector<unsigned char> buffer;
		std::string str;

		for (auto elem : state.rng.getList())
			str += " " + charToHex(elem);
		this->_log("Generated random list is" + str);
		packet.emplace_back();
		packet.push_back(state.rng.getList());

		this->_log("Making packet for trainer " + state.me.name);
		buffer = this->convertString(state.me.name);
		buffer.resize(11, ASCIIToPkmn1CharConversionTable['\0']);
		for (unsigned char &c : buffer)
			if (c == UNAVAILABLE_BYTE)
				c = 0xFF;

		if (state.me.team.empty())
			this->_log("Warning: Team is empty");

		buffer.push_back(state.me.team.size());
		this->_log("Pushing " + std::to_string(state.me.team.size()) + " pokémon(s)");
		for (const Pokemon &pkmn : state.me.team) {
			this->_log(pkmn.dump());
			buffer.push_back(pkmn.getID() != UNAVAILABLE_BYTE ? pkmn.getID() : 0xFF);
		}
		buffer.resize(19, 0xFF);

		for (const Pokemon &pkmn : state.me.team)
			for (unsigned char c : pkmn.encode())
				buffer.push_back(c != UNAVAILABLE_BYTE ? c : 0xFF);
		buffer.resize(19 + 44 * 6, 0);

		packet.push_back(buffer);
		buffer = this->convertString(state.me.name);
		buffer.resize(11, ASCIIToPkmn1CharConversionTable['\0']);
		for (unsigned char c : buffer)
			packet[2].push_back(c != UNAVAILABLE_BYTE ? c : 0xFF);
		packet[2].resize(packet[2].size() + 55, 0);

		for (const Pokemon &pkmn : state.me.team) {
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
				EmulatorGameHandle::_displayPacket(p);
		return packet;
	}

	std::pair<unsigned, unsigned> EmulatorGameHandle::getBattleSendingProgress() const
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
