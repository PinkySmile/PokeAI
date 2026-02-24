//
// Headless smoke-test: feed each example .log file through ShowdownReader and
// print a summary of events produced.  Exits non-zero if any parsing error
// is thrown.
//

#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "ShowdownReader.hpp"

static const char *eventName(const PkmnCommon::Event &e)
{
	if (std::holds_alternative<PkmnCommon::TextEvent>(e))         return "Text";
	if (std::holds_alternative<PkmnCommon::MoveEvent>(e))         return "Move";
	if (std::holds_alternative<PkmnCommon::AnimEvent>(e))         return "Anim";
	if (std::holds_alternative<PkmnCommon::ExtraAnimEvent>(e))    return "ExtraAnim";
	if (std::holds_alternative<PkmnCommon::HealthModEvent>(e))    return "HealthMod";
	if (std::holds_alternative<PkmnCommon::SwitchEvent>(e))       return "Switch";
	if (std::holds_alternative<PkmnCommon::WithdrawEvent>(e))     return "Withdraw";
	if (std::holds_alternative<PkmnCommon::DeathEvent>(e))        return "Death";
	if (std::holds_alternative<PkmnCommon::HitEvent>(e))          return "Hit";
	if (std::holds_alternative<PkmnCommon::StatusClearedEvent>(e))return "StatusCleared";
	if (std::holds_alternative<PkmnCommon::TurnStartEvent>(e))    return "TurnStart";
	if (std::holds_alternative<PkmnCommon::MoveMissEvent>(e))     return "MoveMiss";
	if (std::holds_alternative<PkmnCommon::GameStartEvent>(e))    return "GameStart";
	if (std::holds_alternative<PkmnCommon::GameEndEvent>(e))      return "GameEnd";
	return "Unknown";
}

static int testFile(const std::string &path)
{
	PkmnRenderer::GameState state{};
	memset(&state, 0, sizeof(state));
	ShowdownReader reader{state};

	std::ifstream f(path);
	if (!f) { std::cerr << "Cannot open: " << path << "\n"; return 1; }

	std::string line;
	std::vector<PkmnCommon::Event> events;
	int lineNo = 0;
	int eventTotal = 0;

	// Tally per event type
	std::map<std::string, int> counts;

	try {
		while (std::getline(f, line)) {
			lineNo++;
			reader.feed(events, line);
			for (auto &e : events)
				counts[eventName(e)]++;
			eventTotal += (int)events.size();
			events.clear();
		}
		// Flush last chunk
		reader.feed(events, "|");
		for (auto &e : events)
			counts[eventName(e)]++;
		eventTotal += (int)events.size();
	} catch (const std::exception &ex) {
		std::cerr << "[FAIL] " << path << " (line " << lineNo << "): " << ex.what() << "\n";
		return 1;
	}

	std::cout << "[OK]  " << path << "\n"
	          << "      Lines=" << lineNo << "  Events=" << eventTotal << "\n"
	          << "      P1=" << state.p1.name << "  P2=" << state.p2.name << "\n";
	for (auto &[k, v] : counts)
		std::cout << "        " << k << ": " << v << "\n";
	return 0;
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		std::cerr << "Usage: test_showdown_reader <file.log> [...]\n";
		return 1;
	}
	int rc = 0;
	for (int i = 1; i < argc; i++)
		rc |= testFile(argv[i]);
	return rc;
}
