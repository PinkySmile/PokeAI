//
// Created by PinkySmile on 21/06/25.
//

#include "Team.hpp"
#include "../Exception.hpp"
#include "EmulatorGameHandle.hpp"

namespace PokemonGen1
{
	Trainer loadTrainer(const std::vector<unsigned char> &data, PokemonRandomGenerator &rng, const Pokemon::Logger &logger)
	{
		Trainer result;
		auto it = data.begin();

		if (data.size() != 11 * 7 + 44 * 6 + 1)
			throw InvalidSaveFileException("The data size doesn't match (expected 342B but got " + std::to_string(data.size()) + "B)");
		/* Content */
		result.first = EmulatorGameHandle::convertString(data);
		if (result.first.size() > 10)
			throw InvalidSaveFileException("The trainer name \"" + result.first + "\" is too long (Expected at most 10 characters but gt " + std::to_string(result.first.size()) + ")");
		it += 11;

		unsigned char nbPkmns = *(it++);

		if (nbPkmns > 6 || !nbPkmns)
			throw InvalidSaveFileException("The number of pokémons is invalid (Expected between 1 and 6 pokémons but got " + std::to_string(nbPkmns) + ")");

		auto names = it + Pokemon::ENCODED_SIZE * 6;

		result.second.clear();
		result.second.reserve(nbPkmns);
		for (int i = 0; i < nbPkmns; i++) {
			std::array<unsigned char, Pokemon::ENCODED_SIZE> arr;

			std::copy(it, it + Pokemon::ENCODED_SIZE, arr.begin());
			it += Pokemon::ENCODED_SIZE;
			result.second.emplace_back(rng, logger, EmulatorGameHandle::convertString(std::vector<unsigned char>{names, names + 11}), arr, false);
			names += 11;
		}
		return result;
	}

	std::vector<unsigned char> saveTrainer(const Trainer &trainer)
	{
		std::vector<unsigned char> data;
		std::vector<unsigned char> tmp;

		data.reserve(11 * 7 + 44 * 6 + 1);
		tmp = EmulatorGameHandle::convertString(trainer.first);
		for (size_t i = 0; i < Pokemon::NICK_SIZE; i++)
			if (i < tmp.size())
				data.push_back(tmp[i]);
			else
				data.push_back(ASCIIToPkmn1CharConversionTable['\0']);
		data.push_back(ASCIIToPkmn1CharConversionTable['\0']);

		data.push_back(trainer.second.size());
		for (const Pokemon &pkmn : trainer.second) {
			auto arr = pkmn.encode();

			data.insert(data.end(), arr.begin(), arr.end());
		}
		for (size_t i = trainer.second.size(); i < 6; i++)
			data.resize(data.size() + Pokemon::ENCODED_SIZE, 0);

		for (const Pokemon &pkmn : trainer.second) {
			tmp = EmulatorGameHandle::convertString(pkmn.getNickname());
			for (size_t i = 0; i < Pokemon::NICK_SIZE; i++)
				if (i < tmp.size())
					data.push_back(tmp[i]);
				else
					data.push_back(ASCIIToPkmn1CharConversionTable['\0']);
			data.push_back(ASCIIToPkmn1CharConversionTable['\0']);
		}
		for (size_t i = trainer.second.size(); i < 6; i++)
			data.resize(data.size() + Pokemon::NICK_SIZE + 1, ASCIIToPkmn1CharConversionTable['\0']);
		return data;
	}
}