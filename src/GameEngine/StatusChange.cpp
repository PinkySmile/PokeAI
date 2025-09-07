//
// Created by PinkySmile on 21/07/2019.
//

#include "StatusChange.hpp"

namespace PokemonGen1
{
	std::string statusToString(unsigned status)
	{
		std::vector<const char *> f;
		std::string result;

		if (status == STATUS_NONE)
			return "OK";
		if (status & STATUS_PARALYZED)
			f.push_back("Paralyzed");
		if (status & STATUS_BURNED)
			f.push_back("Burned");
		if (status & STATUS_FROZEN)
			f.push_back("Frozen");
		if (status & STATUS_POISONED)
			f.push_back("Poisoned");
		if (status & STATUS_BADLY_POISONED)
			f.push_back("Badly Poisoned");
		if (status & STATUS_ASLEEP)
			f.push_back("Asleep");
		if (status & STATUS_CONFUSED)
			f.push_back("Confused");
		if (status & STATUS_LEECHED)
			f.push_back("Leeched by Seeds");
		if (status & STATUS_KO)
			f.push_back("KO");
		if (status & STATUS_FLINCHED)
			f.push_back("Flinched");

		if (f.empty())
			return "???";

		result = f[0];
		for (size_t i = 1; i < f.size(); i++) {
			result += "+";
			result += f[i];
		}
		return result;
	}

	std::string statusToStringShort(unsigned status)
	{
		std::vector<const char *> f;
		std::string result;

		if (status == STATUS_NONE)
			return "OK";
		if (status & STATUS_PARALYZED)
			f.push_back("PAR");
		if (status & STATUS_BURNED)
			f.push_back("BRN");
		if (status & STATUS_FROZEN)
			f.push_back("FRZ");
		if (status & STATUS_POISONED)
			f.push_back("PSN");
		if (status & STATUS_BADLY_POISONED)
			f.push_back("BPN");
		if (status & STATUS_ASLEEP)
			f.push_back("SLP");
		if (status & STATUS_CONFUSED)
			f.push_back("CFZ");
		if (status & STATUS_LEECHED)
			f.push_back("LCH");
		if (status & STATUS_KO)
			f.push_back("KO");
		if (status & STATUS_FLINCHED)
			f.push_back("FLN");

		if (f.empty())
			return "???";

		result = f[0];
		for (size_t i = 1; i < f.size(); i++) {
			result += "+";
			result += f[i];
		}
		return result;
	}
}