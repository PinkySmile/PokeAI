#include <iostream>
#include <sstream>
#include <fstream>
#include <random>
#include "EmulatorHandle.hpp"
#include "Exception.hpp"
#include "BgbHandler.hpp"

std::vector<unsigned char> values;
EmulatorHandle *globalHandle = nullptr;

void sigHandler(int)
{
	if (globalHandle)
		globalHandle->disconnect();
	globalHandle = nullptr;
}

int main(int argc, char **argv)
{
	std::mt19937_64 random{static_cast<unsigned long long>(time(NULL))};
	bool isPlayer2 = argc == 1;
	bool done = false;
	unsigned int timer = 0;
	unsigned char buffer = 0;
	int stage = 0;
	std::function<unsigned char (EmulatorHandle &handle, unsigned char byte)> byteHandler =
	[&stage, &timer, &done, &buffer, &isPlayer2, &random](EmulatorHandle &handle, unsigned char byte) {
		if (!stage) {
			if (byte == 1) {
				stage++;
				handle.reply(1 + isPlayer2);
				handle.reply(0);
				handle.reply(0);
				done = false;
				return static_cast<unsigned char>(96);
			}
		} else if (stage == 1) {
			if (byte >= 208)
				stage = 3;
			else if (byte == 0) {
				timer = 5;
				stage++;
			} else if (byte == 96)
				stage++;
		} else if (stage == 2) {
			if (byte >= 208)
				stage++;
			else if (byte == 96)
				return static_cast<unsigned char>(isPlayer2 * 96);
		} else if (stage == 3) {
			if (byte != 254 && byte)
				done = true;
			if (byte == 254) {
				if (buffer >= 3) {
					stage++;
					buffer = 0;
					return static_cast<unsigned char>(0);
				}
				buffer = 0;
			} else if (byte == 2) {
				stage = 0;
				timer = 0;
			} else if (byte == 213) {
				handle.log("User chose colosseum for me");
				stage++;
				buffer = 0;
			} else if (byte >= 212) {
				throw UnexpectedUserActionException("User didn't chose colosseum ( bad user >:( )");
			} else if (byte >= 208) {
				return static_cast<unsigned char>(208);
			} else if (byte == 96) {
				stage--;
				return static_cast<unsigned char>(0);
			}
		} else if (stage == 4) {
			if (buffer < 2)
				buffer++;
			else {
				values.push_back(byte);
				return static_cast<unsigned char>(random() % 256);
			}
		}
		return byte;
	};
	BGBHandler handler(byteHandler, byteHandler, "localhost", 10800, true);

	signal(2, sigHandler);
	globalHandle = &handler;
	while (handler.isConnected()) {
		switch (stage) {
		case 1:
		case 2:
			if (!isPlayer2)
				handler.sendByte(96);
			break;
		case 3:
			if (isPlayer2)
				continue;
			if (timer) {
				buffer = 0;
				timer--;
			} else if (!done) {
				handler.sendByte(96);
			} else {
				handler.sendByte((209 + (buffer >= 6) * 4) * (buffer % 3 != 2));
				buffer++;
				if (buffer >= 9)
					buffer = 3;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	//std::ofstream file{};
	for (unsigned int i = 0; i < values.size(); i += 20) {
		for (unsigned j = 0; j < 20 && j + i < values.size(); j++)
			printf("%02X ", values[j + i]);
		for (int j = 0; j < static_cast<int>(i - values.size() + 20); j++)
			printf("   ");
		for (unsigned j = 0; j < 20 && j + i < values.size(); j++)
			printf("%c", isprint(values[j + i]) ? values[j + i] : '.');
		for (int j = 0; j < static_cast<int>(i - values.size() + 20); j++)
			printf(" ");
		printf("\n");
	}
	return 0;
}