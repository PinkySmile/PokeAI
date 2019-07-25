//
// Created by Gegel85 on 13/07/2019.
//

#include <thread>
#include "PkmnGen1Handle.hpp"
#include "Exception.hpp"

extern std::vector<unsigned char> values;

EmulatorHandle *handler = nullptr;

void sigHandler(int)
{
	if (!handler)
		return;
	handler->disconnect();
	handler = nullptr;
}

namespace Pokemon
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
				printf("%c", isprint(Pkmn1CharToASCIIConversionTable[packet[j + i]]) ? Pkmn1CharToASCIIConversionTable[packet[j + i]] : '.');
			printf("\n");
		}
	}

	PkmnGen1Handle::PkmnGen1Handle(
		const std::function<
			EmulatorHandle *(
				const ByteHandle &byteHandle,
				const std::string &ip,
				unsigned short port
			)
		> &emulatorMaker,
		const std::function<BattleAction(PkmnGen1Handle &)> &battleHandler,
		const std::string &trainerName,
		bool player2
	) :
		_emulatorMaker(emulatorMaker),
		_emulator{nullptr},
		_trainerName(trainerName),
		_isPlayer2(player2),
		_randomGenerator(time(nullptr)),
		_battleHandler(battleHandler)
	{
		if (this->_trainerName.size() > 10) {
			this->_log("Warning: trainer name is too long");
			this->_trainerName = this->_trainerName.substr(0, 10);
		}
		this->_byteHandler = [this](EmulatorHandle &handle, unsigned char byte) {
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
					return static_cast<unsigned char>(208);
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
					this->_received = this->_sent = this->_done = false;
					this->_log("Sending battle data");
					this->_stage = EXCHANGE_POKEMONS;
					this->_receiveBuffer.clear();
				} else if (byte >= 208 && byte != 254){
					this->_log("Oops, we are still choosing room");
					this->_stage = ROOM_CHOOSE;
				}
				break;
			case EXCHANGE_POKEMONS:
				values.push_back(byte);
				this->_receiveBuffer.push_back(byte);
				this->_sent = false;
				this->_done = true;
				if (byte == SYNC_BYTE)
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
				if (this->_done && byte == 0xFE) {
					this->_interpretPacket();
					this->_stage = BATTLE;
					this->_done = false;
					this->_state.pokemonOnField = 0;
					this->_state.opponentPokemonOnField = 0;
					this->_state.team.clear();
					for (Pokemon &pkmn : this->_pkmns)
						this->_state.team.emplace_back(pkmn);
					this->_state.nextAction = NoAction;
					this->_log("Done: going to battle");
				}
				if (byte == SYNC_BYTE)
					return byte;
				this->_received = true;
				break;
			case BATTLE:
				if (byte == 2)
					this->_stage = PKMN_CENTER;
				else if (byte == 0) {
					if (this->_state.nextAction == Run) {
						this->_log("I ran");
						this->_stage = PING_POKEMON_EXCHANGE;
					}
					this->_state.nextAction = NoAction;
					this->_state.nextOpponentAction = NoAction;
				} else
					this->_state.nextOpponentAction = static_cast<BattleAction>(byte);
				switch (byte) {
				case Run:
					this->_log("Opponent ran");
					this->_stage = PING_POKEMON_EXCHANGE;
				}
				break;
			}
			return byte;
		};
	}

	void PkmnGen1Handle::connect(const std::string &ip, unsigned short port)
	{
		if (handler)
			return;
		this->_emulator.reset(this->_emulatorMaker(this->_byteHandler, ip, port));
		this->_log("Connected to " + ip + ":" + std::to_string(port));
		handler = &*this->_emulator;
		while (this->_emulator->isConnected()) {
			switch (this->_stage) {
			case PINGING_OPPONENT:
			case PING_POKEMON_EXCHANGE:
				if (!this->_isPlayer2)
					this->_emulator->sendByte(PING_BYTE);
				std::this_thread::sleep_for(std::chrono::milliseconds(150));
				break;
			case ROOM_CHOOSE:
				if (this->_isPlayer2)
					continue;
				if (this->_timer) {
					this->_buffer = 0;
					this->_timer--;
				} else if (!this->_done) {
					this->_emulator->sendByte(PING_BYTE);
				} else {
					this->_emulator->sendByte((209 + (this->_buffer / 3 % 2) * 4) * (this->_buffer % 3 != 2));
					this->_buffer++;
					if (this->_buffer >= 12)
						this->_buffer = 6;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(150));
				break;
			case EXCHANGE_POKEMONS:
				if (this->_isPlayer2) {
					std::this_thread::sleep_for(std::chrono::milliseconds(1000));
					break;
				}
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
					this->_emulator->sendByte(this->_sendBuffer[this->_sendBufferIndex.first][this->_sendBufferIndex.second++]);
				} else if (this->_sendBufferIndex.first != 3) {
					this->_emulator->sendByte(SYNC_BYTE);
					this->_syncSignalsSent++;
				} else
					this->_emulator->sendByte(0x00);
				break;
			case BATTLE:
				this->_emulator->sendByte(this->_state.nextAction);
				if (this->_state.nextAction == NoAction)
					this->_state.nextAction = this->_battleHandler(*this);
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				break;
			default:
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
	}

	std::vector<unsigned char> PkmnGen1Handle::convertString(const std::string &str)
	{
		std::vector<unsigned char> result;

		for (char c : str)
			result.push_back(ASCIIToPkmn1CharConversionTable[static_cast<unsigned char>(c)]);
		result.push_back(0x50);
		return result;
	}

	std::string PkmnGen1Handle::convertString(const std::vector<unsigned char> &str)
	{
		std::string result;

		for (unsigned char c : str) {
			if (!Pkmn1CharToASCIIConversionTable[c])
				break;
			result.push_back(Pkmn1CharToASCIIConversionTable[c]);
		}
		return result;
	}

	void PkmnGen1Handle::_interpretPacket()
	{
		this->_log("Decrypting received packet");
		displayPacket(values);
		values.clear();

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

		this->_log("He has " + std::to_string(nbPkmns) + " pokémon(s)");
		this->_receiveBuffer.erase(this->_receiveBuffer.begin());
		for (int i = 0; i < nbPkmns; i++)
			this->_log("Pokémon " + std::to_string(i) + ": " + pokemonList[this->_receiveBuffer[i]].name);
		this->_receiveBuffer.erase(this->_receiveBuffer.begin(), this->_receiveBuffer.begin() + 7);

		std::vector<unsigned char> pkmnData{this->_receiveBuffer.begin(), this->_receiveBuffer.begin() + 44 * 6};

		this->_receiveBuffer.erase(this->_receiveBuffer.begin(), this->_receiveBuffer.begin() + 44 * 6 + 66);
		this->_state.opponentTeam.clear();
		for (int i = 0; i < nbPkmns; i++) {
			this->_state.opponentTeam.emplace_back(this->_randomGenerator, this->convertString(this->_receiveBuffer), pkmnData);
			this->_receiveBuffer.erase(this->_receiveBuffer.begin(), this->_receiveBuffer.begin() + 11);
			pkmnData.erase(pkmnData.begin(), pkmnData.begin() + 44);
		}
		for (Pokemon &pkmn : this->_state.opponentTeam)
			this->_log(pkmn.dump());
	}

	void PkmnGen1Handle::_log(const std::string &msg)
	{
		std::cout << "[PkmnGen1Handle]: " << msg << std::endl;
	}

	std::vector<std::vector<unsigned char>> PkmnGen1Handle::_craftPacket()
	{
		std::vector<std::vector<unsigned char>> packet;
		std::vector<unsigned char> buffer;

		packet.emplace_back();
		packet.push_back(HEADER_PACKET);

		this->_log("Making packet for trainer " + this->_trainerName);
		buffer = this->convertString(this->_trainerName);
		buffer.resize(11, '\x50');

		buffer.push_back(this->_pkmns.size());
		this->_log("Pushing " + std::to_string(this->_pkmns.size()) + " pokémon(s)");
		for (Pokemon &pkmn : this->_pkmns) {
			this->_log(pkmn.dump());
			buffer.push_back(pkmn.getID());
		}
		buffer.resize(19, 0xFF);
		for (size_t i = 0; i < 6; i++)
			for (unsigned char c : (i < this->_pkmns.size() ? this->_pkmns[i].encode() : NO_PKMN))
				buffer.push_back(c);

		packet.push_back(buffer);
		buffer = this->convertString(this->_trainerName);
		buffer.resize(11, '\x50');
		for (unsigned char c : buffer)
			packet[2].push_back(c);
		for (unsigned char c : MIDDLE_PACKET)
			packet[2].push_back(c);

		buffer.clear();
		for (Pokemon &pkmn : this->_pkmns) {
			std::vector<unsigned char> buf = this->convertString(pkmn.getName());

			buf.resize(11, '\x50');
			buf[10] = ASCIIToPkmn1CharConversionTable['\0'];
			for (unsigned char c : buf)
				buffer.push_back(c);
		}
		buffer.resize(66, '\x50');
		for (unsigned char c : buffer)
			packet[2].push_back(c);
		packet[2].push_back(0xA3);
		packet[2].push_back(0xFF);
		packet[2].push_back(0xFF);
		packet[2].push_back(0xFF);
		packet.push_back(PACKET_FOOTER);

		this->_log("Packets are ready");
		for (auto &p : packet)
			displayPacket(p);
		return packet;
	}

	/*
	** From Rhydon
	** https://github.com/SciresM/Rhydon/blob/2056e8f044d3c5178ad2d697d0823d2b799bb099/Rhydon/Encoding.cs#L122
	*/
	const std::vector<unsigned char> ASCIIToPkmn1CharConversionTable{
		0x50, /* 0x00 */
		0x00, /* 0x01 */
		0x00, /* 0x02 */
		0x00, /* 0x03 */
		0x00, /* 0x04 */
		0x00, /* 0x05 */
		0x00, /* 0x06 */
		0x00, /* 0x07 */
		0x00, /* 0x08 */
		0x00, /* 0x09 */
		0x00, /* 0x0A */
		0x00, /* 0x0B */
		0x00, /* 0x0C */
		0x00, /* 0x0D */
		0x00, /* 0x0E */
		0x00, /* 0x0F */
		0x00, /* 0x10 */
		0x00, /* 0x11 */
		0x00, /* 0x12 */
		0x00, /* 0x13 */
		0x00, /* 0x14 */
		0x00, /* 0x15 */
		0x00, /* 0x16 */
		0x00, /* 0x17 */
		0x00, /* 0x18 */
		0x00, /* 0x19 */
		0x00, /* 0x1A */
		0x00, /* 0x1B */
		0x00, /* 0x1C */
		0x00, /* 0x1D */
		0x00, /* 0x1E */
		0x00, /* 0x1F */
		0x7F, /* 0x20 */
		0xE7, /* 0x21 */
		0x00, /* 0x22 */
		0x00, /* 0x23 */
		0x00, /* 0x24 */
		0x00, /* 0x25 */
		0x00, /* 0x26 */
		0x00, /* 0x27 */
		0x9A, /* 0x28 */
		0x9B, /* 0x29 */
		0x00, /* 0x2A */
		0x00, /* 0x2B */
		0xF4, /* 0x2C */
		0xE3, /* 0x2D */
		0xF2, /* 0x2E */
		0xF3, /* 0x2F */
		0xF6, /* 0x30 */
		0xF7, /* 0x31 */
		0xF8, /* 0x32 */
		0xF9, /* 0x33 */
		0xFA, /* 0x34 */
		0xFB, /* 0x35 */
		0xFC, /* 0x36 */
		0xFD, /* 0x37 */
		0xFE, /* 0x38 */
		0xFF, /* 0x39 */
		0x9C, /* 0x3A */
		0x9D, /* 0x3B */
		0x00, /* 0x3C */
		0x00, /* 0x3D */
		0x00, /* 0x3E */
		0xE6, /* 0x3F */
		0x00, /* 0x40 */
		0x80, /* 0x41 */
		0x81, /* 0x42 */
		0x82, /* 0x43 */
		0x83, /* 0x44 */
		0x84, /* 0x45 */
		0x85, /* 0x46 */
		0x86, /* 0x47 */
		0x87, /* 0x48 */
		0x88, /* 0x49 */
		0x89, /* 0x4A */
		0x8A, /* 0x4B */
		0x8B, /* 0x4C */
		0x8C, /* 0x4D */
		0x8D, /* 0x4E */
		0x8E, /* 0x4F */
		0x8F, /* 0x50 */
		0x90, /* 0x51 */
		0x91, /* 0x52 */
		0x92, /* 0x53 */
		0x93, /* 0x54 */
		0x94, /* 0x55 */
		0x95, /* 0x56 */
		0x96, /* 0x57 */
		0x97, /* 0x58 */
		0x98, /* 0x59 */
		0x99, /* 0x5A */
		0x9E, /* 0x5B */
		0x00, /* 0x5C */
		0x9F, /* 0x5D */
		0x00, /* 0x5E */
		0x00, /* 0x5F */
		0xF5, /* 0x60 */
		0xA0, /* 0x61 */
		0xA1, /* 0x62 */
		0xA2, /* 0x63 */
		0xA3, /* 0x64 */
		0xA4, /* 0x65 */
		0xA5, /* 0x66 */
		0xA6, /* 0x67 */
		0xA7, /* 0x68 */
		0xA8, /* 0x69 */
		0xA9, /* 0x6A */
		0xAA, /* 0x6B */
		0xAB, /* 0x6C */
		0xAC, /* 0x6D */
		0xAD, /* 0x6E */
		0xAE, /* 0x6F */
		0xAF, /* 0x70 */
		0xB0, /* 0x71 */
		0xB1, /* 0x72 */
		0xB2, /* 0x73 */
		0xB3, /* 0x74 */
		0xB4, /* 0x75 */
		0xB5, /* 0x76 */
		0xB6, /* 0x77 */
		0xB7, /* 0x78 */
		0xB8, /* 0x79 */
		0xB9, /* 0x7A */
		0xE1, /* 0x7B */
		0x00, /* 0x7C */
		0xE2, /* 0x7D */
		0xEF, /* 0x7E */
		0x00, /* 0x7F */
		0x00, /* 0x80 */
		0x00, /* 0x81 */
		0x00, /* 0x82 */
		0x00, /* 0x83 */
		0x00, /* 0x84 */
		0x00, /* 0x85 */
		0x00, /* 0x86 */
		0x00, /* 0x87 */
		0x00, /* 0x88 */
		0x00, /* 0x89 */
		0x00, /* 0x8A */
		0x00, /* 0x8B */
		0x00, /* 0x8C */
		0x00, /* 0x8D */
		0x00, /* 0x8E */
		0x00, /* 0x8F */
		0x00, /* 0x90 */
		0x00, /* 0x91 */
		0x00, /* 0x92 */
		0x00, /* 0x93 */
		0x00, /* 0x94 */
		0x00, /* 0x95 */
		0x00, /* 0x96 */
		0x00, /* 0x97 */
		0x00, /* 0x98 */
		0x00, /* 0x99 */
		0x00, /* 0x9A */
		0x00, /* 0x9B */
		0x00, /* 0x9C */
		0x00, /* 0x9D */
		0x00, /* 0x9E */
		0x00, /* 0x9F */
		0x00, /* 0xA0 */
		0x00, /* 0xA1 */
		0x00, /* 0xA2 */
		0x00, /* 0xA3 */
		0x00, /* 0xA4 */
		0x00, /* 0xA5 */
		0x00, /* 0xA6 */
		0x00, /* 0xA7 */
		0x00, /* 0xA8 */
		0x00, /* 0xA9 */
		0x00, /* 0xAA */
		0x00, /* 0xAB */
		0x00, /* 0xAC */
		0x00, /* 0xAD */
		0x00, /* 0xAE */
		0x00, /* 0xAF */
		0x00, /* 0xB0 */
		0x00, /* 0xB1 */
		0x00, /* 0xB2 */
		0x00, /* 0xB3 */
		0x00, /* 0xB4 */
		0x00, /* 0xB5 */
		0x00, /* 0xB6 */
		0x00, /* 0xB7 */
		0x00, /* 0xB8 */
		0x00, /* 0xB9 */
		0x00, /* 0xBA */
		0x00, /* 0xBB */
		0x00, /* 0xBC */
		0x00, /* 0xBD */
		0x00, /* 0xBE */
		0x00, /* 0xBF */
		0x00, /* 0xC0 */
		0x00, /* 0xC1 */
		0x00, /* 0xC2 */
		0x00, /* 0xC3 */
		0x00, /* 0xC4 */
		0x00, /* 0xC5 */
		0x00, /* 0xC6 */
		0x00, /* 0xC7 */
		0x00, /* 0xC8 */
		0x00, /* 0xC9 */
		0x00, /* 0xCA */
		0x00, /* 0xCB */
		0x00, /* 0xCC */
		0x00, /* 0xCD */
		0x00, /* 0xCE */
		0x00, /* 0xCF */
		0x00, /* 0xD0 */
		0x00, /* 0xD1 */
		0x00, /* 0xD2 */
		0x00, /* 0xD3 */
		0x00, /* 0xD4 */
		0x00, /* 0xD5 */
		0x00, /* 0xD6 */
		0x00, /* 0xD7 */
		0x00, /* 0xD8 */
		0x00, /* 0xD9 */
		0x00, /* 0xDA */
		0x00, /* 0xDB */
		0x00, /* 0xDC */
		0x00, /* 0xDD */
		0x00, /* 0xDE */
		0x00, /* 0xDF */
		0x00, /* 0xE0 */
		0x00, /* 0xE1 */
		0x00, /* 0xE2 */
		0x00, /* 0xE3 */
		0x00, /* 0xE4 */
		0x00, /* 0xE5 */
		0x00, /* 0xE6 */
		0x00, /* 0xE7 */
		0x00, /* 0xE8 */
		0x00, /* 0xE9 */
		0x00, /* 0xEA */
		0x00, /* 0xEB */
		0x00, /* 0xEC */
		0x00, /* 0xED */
		0x00, /* 0xEE */
		0x00, /* 0xEF */
		0x00, /* 0xF0 */
		0x00, /* 0xF1 */
		0x00, /* 0xF2 */
		0x00, /* 0xF3 */
		0x00, /* 0xF4 */
		0x00, /* 0xF5 */
		0x00, /* 0xF6 */
		0x00, /* 0xF7 */
		0x00, /* 0xF8 */
		0x00, /* 0xF9 */
		0x00, /* 0xFA */
		0x00, /* 0xFB */
		0x00, /* 0xFC */
		0x00, /* 0xFD */
		0x5D, /* 0xFE */
		0x00  /* 0xFF */
	};

	/*
	** From Rhydon
	** https://github.com/SciresM/Rhydon/blob/2056e8f044d3c5178ad2d697d0823d2b799bb099/Rhydon/Encoding.cs#L381
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
		'[',          /* 0x9A */
		'\\',         /* 0x9B */
		']',          /* 0x9C */
		'^',          /* 0x9D */
		'_',          /* 0x9E */
		'`',          /* 0x9F */
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
		CHAR_INVALID, /* 0xBA */
		CHAR_INVALID, /* 0xBB */
		CHAR_INVALID, /* 0xBC */
		CHAR_INVALID, /* 0xBD */
		CHAR_INVALID, /* 0xBE */
		CHAR_INVALID, /* 0xBF */
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
		CHAR_INVALID, /* 0xE0 */
		CHAR_PK_NUM,  /* 0xE1 */
		CHAR_MN_NUM,  /* 0xE2 */
		'-',          /* 0xE3 */
		CHAR_INVALID, /* 0xE4 */
		CHAR_INVALID, /* 0xE5 */
		'?',          /* 0xE6 */
		'!',          /* 0xE7 */
		'.',          /* 0xE8 */
		CHAR_INVALID, /* 0xE9 */
		CHAR_INVALID, /* 0xEA */
		CHAR_INVALID, /* 0xEB */
		CHAR_INVALID, /* 0xEC */
		CHAR_INVALID, /* 0xED */
		CHAR_INVALID, /* 0xEE */
		CHAR_MAL_NUM, /* 0xEF */
		CHAR_INVALID, /* 0xF0 */
		CHAR_INVALID, /* 0xF1 */
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