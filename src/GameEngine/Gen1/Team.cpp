//
// Created by PinkySmile on 21/06/25.
//

#include <cstring>
#include "../Exception.hpp"
#include "Team.hpp"

namespace PokemonGen1
{
	Trainer loadTrainer(const std::vector<unsigned char> &data, BattleState &state)
	{
		Trainer result;
		auto it = data.begin();

		if (data.size() != TRAINER_DATA_SIZE)
			throw InvalidSaveFileException("The data size doesn't match (expected 342B but got " + std::to_string(data.size()) + "B)");
		/* Content */
		result.first = std::string(data.begin(), data.begin() + 11);
		result.first.resize(strlen(result.first.c_str()));
		if (result.first.size() > 10)
			throw InvalidSaveFileException("Invalid trainer name.");
		it += 11;

		unsigned char nbPkmns = *(it++);

		if (nbPkmns > 6 || !nbPkmns)
			throw InvalidSaveFileException("The number of pokémons is invalid (Expected between 1 and 6 pokémons but got " + std::to_string(nbPkmns) + ")");

		auto names = it + Pokemon::ENCODED_SIZE * 6;

		result.second.clear();
		result.second.reserve(nbPkmns);
		for (int i = 0; i < nbPkmns; i++) {
			std::array<unsigned char, Pokemon::ENCODED_SIZE> arr;
			std::string tmp{names, names + 11};

			std::copy(it, it + Pokemon::ENCODED_SIZE, arr.begin());
			tmp.resize(strlen(tmp.c_str()));
			it += Pokemon::ENCODED_SIZE;
			result.second.emplace_back(state, tmp, arr, false);
			names += 11;
		}
		return result;
	}

	std::vector<unsigned char> saveTrainer(const Trainer &trainer)
	{
		std::vector<unsigned char> data;
		std::string tmp;

		data.reserve(11 * 7 + 44 * 6 + 1);
		for (size_t i = 0; i < Pokemon::NICK_SIZE; i++)
			if (i < trainer.first.size())
				data.push_back(trainer.first[i]);
			else
				data.push_back('\0');
		data.push_back('\0');

		data.push_back(trainer.second.size());
		for (const Pokemon &pkmn : trainer.second) {
			auto arr = pkmn.encode();

			data.insert(data.end(), arr.begin(), arr.end());
		}
		for (size_t i = trainer.second.size(); i < 6; i++)
			data.resize(data.size() + Pokemon::ENCODED_SIZE, 0);

		for (const Pokemon &pkmn : trainer.second) {
			tmp = pkmn.getNickname();
			for (size_t i = 0; i < Pokemon::NICK_SIZE; i++)
				if (i < tmp.size())
					data.push_back(tmp[i]);
				else
					data.push_back('\0');
			data.push_back('\0');
		}
		for (size_t i = trainer.second.size(); i < 6; i++)
			data.resize(data.size() + Pokemon::NICK_SIZE + 1, '\0');
		return data;
	}
}