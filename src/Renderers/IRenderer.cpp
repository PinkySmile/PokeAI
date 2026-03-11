//
// Created by PinkySmile on 19/11/2025.
//

#include <cstring>
#include "IRenderer.hpp"
#include <SFML/Graphics/Image.hpp>

static std::unordered_map<unsigned, PkmnCommon::PokemonSpecies> gen1SpeciesMap{
	{PokemonGen1::Rhydon,     PkmnCommon::Rhydon},
	{PokemonGen1::Kangaskhan, PkmnCommon::Kangaskhan},
	{PokemonGen1::Nidoran_M,  PkmnCommon::Nidoran_M},
	{PokemonGen1::Clefairy,   PkmnCommon::Clefairy},
	{PokemonGen1::Spearow,    PkmnCommon::Spearow},
	{PokemonGen1::Voltorb,    PkmnCommon::Voltorb},
	{PokemonGen1::Nidoking,   PkmnCommon::Nidoking},
	{PokemonGen1::Slowbro,    PkmnCommon::Slowbro},
	{PokemonGen1::Ivysaur,    PkmnCommon::Ivysaur},
	{PokemonGen1::Exeggutor,  PkmnCommon::Exeggutor},
	{PokemonGen1::Lickitung,  PkmnCommon::Lickitung},
	{PokemonGen1::Exeggcute,  PkmnCommon::Exeggcute},
	{PokemonGen1::Grimer,     PkmnCommon::Grimer},
	{PokemonGen1::Gengar,     PkmnCommon::Gengar},
	{PokemonGen1::Nidoran_F,  PkmnCommon::Nidoran_F},
	{PokemonGen1::Nidoqueen,  PkmnCommon::Nidoqueen},
	{PokemonGen1::Cubone,     PkmnCommon::Cubone},
	{PokemonGen1::Rhyhorn,    PkmnCommon::Rhyhorn},
	{PokemonGen1::Lapras,     PkmnCommon::Lapras},
	{PokemonGen1::Arcanine,   PkmnCommon::Arcanine},
	{PokemonGen1::Mew,        PkmnCommon::Mew},
	{PokemonGen1::Gyarados,   PkmnCommon::Gyarados},
	{PokemonGen1::Shellder,   PkmnCommon::Shellder},
	{PokemonGen1::Tentacool,  PkmnCommon::Tentacool},
	{PokemonGen1::Gastly,     PkmnCommon::Gastly},
	{PokemonGen1::Scyther,    PkmnCommon::Scyther},
	{PokemonGen1::Staryu,     PkmnCommon::Staryu},
	{PokemonGen1::Blastoise,  PkmnCommon::Blastoise},
	{PokemonGen1::Pinsir,     PkmnCommon::Pinsir},
	{PokemonGen1::Tangela,    PkmnCommon::Tangela},
	{PokemonGen1::Growlithe,  PkmnCommon::Growlithe},
	{PokemonGen1::Onix,       PkmnCommon::Onix},
	{PokemonGen1::Fearow,     PkmnCommon::Fearow},
	{PokemonGen1::Pidgey,     PkmnCommon::Pidgey},
	{PokemonGen1::Slowpoke,   PkmnCommon::Slowpoke},
	{PokemonGen1::Kadabra,    PkmnCommon::Kadabra},
	{PokemonGen1::Graveler,   PkmnCommon::Graveler},
	{PokemonGen1::Chansey,    PkmnCommon::Chansey},
	{PokemonGen1::Machoke,    PkmnCommon::Machoke},
	{PokemonGen1::Mr_Mime,    PkmnCommon::Mr_Mime},
	{PokemonGen1::Hitmonlee,  PkmnCommon::Hitmonlee},
	{PokemonGen1::Hitmonchan, PkmnCommon::Hitmonchan},
	{PokemonGen1::Arbok,      PkmnCommon::Arbok},
	{PokemonGen1::Parasect,   PkmnCommon::Parasect},
	{PokemonGen1::Psyduck,    PkmnCommon::Psyduck},
	{PokemonGen1::Drowzee,    PkmnCommon::Drowzee},
	{PokemonGen1::Golem,      PkmnCommon::Golem},
	{PokemonGen1::Magmar,     PkmnCommon::Magmar},
	{PokemonGen1::Electabuzz, PkmnCommon::Electabuzz},
	{PokemonGen1::Magneton,   PkmnCommon::Magneton},
	{PokemonGen1::Koffing,    PkmnCommon::Koffing},
	{PokemonGen1::Mankey,     PkmnCommon::Mankey},
	{PokemonGen1::Seel,       PkmnCommon::Seel},
	{PokemonGen1::Diglett,    PkmnCommon::Diglett},
	{PokemonGen1::Tauros,     PkmnCommon::Tauros},
	{PokemonGen1::Farfetchd,  PkmnCommon::Farfetchd},
	{PokemonGen1::Venonat,    PkmnCommon::Venonat},
	{PokemonGen1::Dragonite,  PkmnCommon::Dragonite},
	{PokemonGen1::Doduo,      PkmnCommon::Doduo},
	{PokemonGen1::Poliwag,    PkmnCommon::Poliwag},
	{PokemonGen1::Jynx,       PkmnCommon::Jynx},
	{PokemonGen1::Moltres,    PkmnCommon::Moltres},
	{PokemonGen1::Articuno,   PkmnCommon::Articuno},
	{PokemonGen1::Zapdos,     PkmnCommon::Zapdos},
	{PokemonGen1::Ditto,      PkmnCommon::Ditto},
	{PokemonGen1::Meowth,     PkmnCommon::Meowth},
	{PokemonGen1::Krabby,     PkmnCommon::Krabby},
	{PokemonGen1::Vulpix,     PkmnCommon::Vulpix},
	{PokemonGen1::Ninetales,  PkmnCommon::Ninetales},
	{PokemonGen1::Pikachu,    PkmnCommon::Pikachu},
	{PokemonGen1::Raichu,     PkmnCommon::Raichu},
	{PokemonGen1::Dratini,    PkmnCommon::Dratini},
	{PokemonGen1::Dragonair,  PkmnCommon::Dragonair},
	{PokemonGen1::Kabuto,     PkmnCommon::Kabuto},
	{PokemonGen1::Kabutops,   PkmnCommon::Kabutops},
	{PokemonGen1::Horsea,     PkmnCommon::Horsea},
	{PokemonGen1::Seadra,     PkmnCommon::Seadra},
	{PokemonGen1::Sandshrew,  PkmnCommon::Sandshrew},
	{PokemonGen1::Sandslash,  PkmnCommon::Sandslash},
	{PokemonGen1::Omanyte,    PkmnCommon::Omanyte},
	{PokemonGen1::Omastar,    PkmnCommon::Omastar},
	{PokemonGen1::Jigglypuff, PkmnCommon::Jigglypuff},
	{PokemonGen1::Wigglytuff, PkmnCommon::Wigglytuff},
	{PokemonGen1::Eevee,      PkmnCommon::Eevee},
	{PokemonGen1::Flareon,    PkmnCommon::Flareon},
	{PokemonGen1::Jolteon,    PkmnCommon::Jolteon},
	{PokemonGen1::Vaporeon,   PkmnCommon::Vaporeon},
	{PokemonGen1::Machop,     PkmnCommon::Machop},
	{PokemonGen1::Zubat,      PkmnCommon::Zubat},
	{PokemonGen1::Ekans,      PkmnCommon::Ekans},
	{PokemonGen1::Paras,      PkmnCommon::Paras},
	{PokemonGen1::Poliwhirl,  PkmnCommon::Poliwhirl},
	{PokemonGen1::Poliwrath,  PkmnCommon::Poliwrath},
	{PokemonGen1::Weedle,     PkmnCommon::Weedle},
	{PokemonGen1::Kakuna,     PkmnCommon::Kakuna},
	{PokemonGen1::Beedrill,   PkmnCommon::Beedrill},
	{PokemonGen1::Dodrio,     PkmnCommon::Dodrio},
	{PokemonGen1::Primeape,   PkmnCommon::Primeape},
	{PokemonGen1::Dugtrio,    PkmnCommon::Dugtrio},
	{PokemonGen1::Venomoth,   PkmnCommon::Venomoth},
	{PokemonGen1::Dewgong,    PkmnCommon::Dewgong},
	{PokemonGen1::Caterpie,   PkmnCommon::Caterpie},
	{PokemonGen1::Metapod,    PkmnCommon::Metapod},
	{PokemonGen1::Butterfree, PkmnCommon::Butterfree},
	{PokemonGen1::Machamp,    PkmnCommon::Machamp},
	{PokemonGen1::Golduck,    PkmnCommon::Golduck},
	{PokemonGen1::Hypno,      PkmnCommon::Hypno},
	{PokemonGen1::Golbat,     PkmnCommon::Golbat},
	{PokemonGen1::Mewtwo,     PkmnCommon::Mewtwo},
	{PokemonGen1::Snorlax,    PkmnCommon::Snorlax},
	{PokemonGen1::Magikarp,   PkmnCommon::Magikarp},
	{PokemonGen1::Muk,        PkmnCommon::Muk},
	{PokemonGen1::Kingler,    PkmnCommon::Kingler},
	{PokemonGen1::Cloyster,   PkmnCommon::Cloyster},
	{PokemonGen1::Electrode,  PkmnCommon::Electrode},
	{PokemonGen1::Clefable,   PkmnCommon::Clefable},
	{PokemonGen1::Weezing,    PkmnCommon::Weezing},
	{PokemonGen1::Persian,    PkmnCommon::Persian},
	{PokemonGen1::Marowak,    PkmnCommon::Marowak},
	{PokemonGen1::Haunter,    PkmnCommon::Haunter},
	{PokemonGen1::Abra,       PkmnCommon::Abra},
	{PokemonGen1::Alakazam,   PkmnCommon::Alakazam},
	{PokemonGen1::Pidgeotto,  PkmnCommon::Pidgeotto},
	{PokemonGen1::Pidgeot,    PkmnCommon::Pidgeot},
	{PokemonGen1::Starmie,    PkmnCommon::Starmie},
	{PokemonGen1::Bulbasaur,  PkmnCommon::Bulbasaur},
	{PokemonGen1::Venusaur,   PkmnCommon::Venusaur},
	{PokemonGen1::Tentacruel, PkmnCommon::Tentacruel},
	{PokemonGen1::Goldeen,    PkmnCommon::Goldeen},
	{PokemonGen1::Seaking,    PkmnCommon::Seaking},
	{PokemonGen1::Ponyta,     PkmnCommon::Ponyta},
	{PokemonGen1::Rapidash,   PkmnCommon::Rapidash},
	{PokemonGen1::Rattata,    PkmnCommon::Rattata},
	{PokemonGen1::Raticate,   PkmnCommon::Raticate},
	{PokemonGen1::Nidorino,   PkmnCommon::Nidorino},
	{PokemonGen1::Nidorina,   PkmnCommon::Nidorina},
	{PokemonGen1::Geodude,    PkmnCommon::Geodude},
	{PokemonGen1::Porygon,    PkmnCommon::Porygon},
	{PokemonGen1::Aerodactyl, PkmnCommon::Aerodactyl},
	{PokemonGen1::Magnemite,  PkmnCommon::Magnemite},
	{PokemonGen1::Charmander, PkmnCommon::Charmander},
	{PokemonGen1::Squirtle,   PkmnCommon::Squirtle},
	{PokemonGen1::Charmeleon, PkmnCommon::Charmeleon},
	{PokemonGen1::Wartortle,  PkmnCommon::Wartortle},
	{PokemonGen1::Charizard,  PkmnCommon::Charizard},
	{PokemonGen1::Oddish,     PkmnCommon::Oddish},
	{PokemonGen1::Gloom,      PkmnCommon::Gloom},
	{PokemonGen1::Vileplume,  PkmnCommon::Vileplume},
	{PokemonGen1::Bellsprout, PkmnCommon::Bellsprout},
	{PokemonGen1::Weepinbell, PkmnCommon::Weepinbell},
	{PokemonGen1::Victreebel, PkmnCommon::Victreebel},
	{PokemonGen1::Missingno,  PkmnCommon::Missingno},
	{182,                     PkmnCommon::Gen1_Fossil1},
	{183,                     PkmnCommon::Gen1_Fossil2},
	{184,                     PkmnCommon::Gen1_Spectre},
};

namespace PkmnRenderer
{
	unsigned IRenderer::getTurn() const
	{
		return this->_currentTurn;
	}

	void IRenderer::goToTurn(unsigned int turn)
	{
		while (this->_currentTurn < turn && !this->_queue.empty())
			this->nextTurn();
		while (this->_currentTurn > turn)
			this->previousTurn();
	}

	void IRenderer::previousTurn()
	{
		if (this->_snapshots.empty())
			return;

		const SavedState &snapshot = this->_snapshots.back();

		this->_currentTurn = snapshot.turn;
		this->_queue = snapshot.queue;
		this->state = snapshot.state;
		this->_snapshots.pop_back();
	}

	void IRenderer::nextTurn()
	{
		bool oldSD = this->soundDisabled;
		unsigned oldT = this->_currentTurn;

		this->soundDisabled = true;
		this->_skipping = true;
		while (oldT == this->_currentTurn && (!this->_queue.empty() || !this->_finished))
			this->update();
		this->_skipping = false;
		this->soundDisabled = oldSD;
	}

	std::vector<unsigned char> IRenderer::renderVec()
	{
		std::vector<unsigned char> vec;

		vec.resize(this->renderBuff(nullptr));
		this->renderBuff(vec.data());
		return vec;
	}

	size_t IRenderer::renderBuff(unsigned char *buffer)
	{
		sf::Vector2u size = this->getSize();
		size_t lineBytes = size.x * 4;
		size_t bytes = lineBytes * size.y;

		if (buffer) {
			if (this->_buffer.getSize() != size)
				(void)this->_buffer.resize(size);
			this->render(this->_buffer);

			const sf::Image img = this->_buffer.getTexture().copyToImage();
			const unsigned char *ptrIn = img.getPixelsPtr();
			unsigned char *ptrOut = buffer + bytes;

			// TODO: For some reason when displaying on an sf::RenderTexture the image is upside down?
			//       All the Gen1Renderer also expects this behaviour but that doesn't look normal
			//       We just reverse it again here.
			for (size_t i = 0; i < size.y; i++) {
				ptrOut -= lineBytes;
				memcpy(ptrOut, ptrIn, lineBytes);
				ptrIn += lineBytes;
			}
		}
		return bytes;
	}

	bool IRenderer::hasAnimationEnded() const
	{
		return this->_finished;
	}

	PlayerState &IRenderer::getState(bool player)
	{
		if (!this->swapSide == player)
			return this->state.p1;
		return this->state.p2;
	}

	void IRenderer::consumeEvent(const PkmnCommon::Event &event)
	{
		this->_queue.push_back(event);
		for (auto &s : this->_snapshots)
			s.queue.push_back(event);
	}

	PkmnCommon::PokemonSpecies gen1SpeciesToCommon(unsigned id)
	{
		if (gen1SpeciesMap.contains(id))
			return gen1SpeciesMap.at(id);
		return PkmnCommon::Missingno;
	}

	GameState fromGen1(const PokemonGen1::BattleState &state)
	{
		GameState result;

		strcpy(result.p1.name, state.me.name.c_str());
		result.p1.substitute = state.me.team[state.me.pokemonOnField].hasSubstitute();
		result.p1.active = state.me.pokemonOnField;
		result.p1.spriteId = gen1SpeciesToCommon(state.me.team[state.me.pokemonOnField].getID());
		result.p1.hidden = false;
		result.p1.acidArmor = false;
		result.p1.exploded = false;
		for (unsigned i = 0; i < std::size(result.p1.team) && i < state.me.team.size(); i++) {
			auto &po = result.p1.team[i];
			auto &pi = state.me.team[i];
			auto &moveSet = pi.getMoveSet();

			po.id = gen1SpeciesToCommon(pi.getID());
			po.hp = pi.getHealth();
			po.maxHp = pi.getMaxHealth();
			po.atk = pi.getRawAttack();
			po.def = pi.getRawDefense();
			po.spd = pi.getRawSpeed();
			po.spe = pi.getRawSpecial();
			po.level = pi.getLevel();
			strcpy(po.name, pi.getName(false).c_str());
			po.asleep = pi.hasStatus(PokemonGen1::STATUS_ASLEEP);
			po.frozen = pi.hasStatus(PokemonGen1::STATUS_FROZEN);
			po.burned = pi.hasStatus(PokemonGen1::STATUS_BURNED);
			po.poisoned = pi.hasStatus(PokemonGen1::STATUS_POISONED);
			po.toxicPoisoned = pi.hasStatus(PokemonGen1::STATUS_BAD_POISON);
			po.paralyzed = pi.hasStatus(PokemonGen1::STATUS_PARALYZED);
			po.ko = pi.hasStatus(PokemonGen1::STATUS_KO);
			po.leeched = pi.hasStatus(PokemonGen1::STATUS_LEECHED);
			po.confused = pi.hasStatus(PokemonGen1::STATUS_CONFUSED);
			for (unsigned j = 0; j < std::size(po.moves) && j < moveSet.size(); j++) {
				po.moves[j].id = moveSet[j].getID();
				po.moves[j].pp = moveSet[j].getPP();
				po.moves[j].maxPp = moveSet[j].getMaxPP();
			}
		}

		strcpy(result.p2.name, state.op.name.c_str());
		result.p2.substitute = state.op.team[state.op.pokemonOnField].hasSubstitute();
		result.p2.active = state.op.pokemonOnField;
		result.p2.spriteId = gen1SpeciesToCommon(state.op.team[state.me.pokemonOnField].getID());
		result.p2.hidden = false;
		result.p2.acidArmor = false;
		result.p2.exploded = false;
		for (unsigned i = 0; i < std::size(result.p2.team) && i < state.op.team.size(); i++) {
			auto &po = result.p2.team[i];
			auto &pi = state.op.team[i];
			auto &moveSet = pi.getMoveSet();

			po.id = gen1SpeciesToCommon(pi.getID());
			po.hp = pi.getHealth();
			po.maxHp = pi.getMaxHealth();
			po.atk = pi.getRawAttack();
			po.def = pi.getRawDefense();
			po.spd = pi.getRawSpeed();
			po.spe = pi.getRawSpecial();
			po.level = pi.getLevel();
			strcpy(po.name, pi.getName(false).c_str());
			po.asleep = pi.hasStatus(PokemonGen1::STATUS_ASLEEP);
			po.frozen = pi.hasStatus(PokemonGen1::STATUS_FROZEN);
			po.burned = pi.hasStatus(PokemonGen1::STATUS_BURNED);
			po.poisoned = pi.hasStatus(PokemonGen1::STATUS_POISONED);
			po.toxicPoisoned = pi.hasStatus(PokemonGen1::STATUS_BAD_POISON);
			po.paralyzed = pi.hasStatus(PokemonGen1::STATUS_PARALYZED);
			po.ko = pi.hasStatus(PokemonGen1::STATUS_KO);
			po.leeched = pi.hasStatus(PokemonGen1::STATUS_LEECHED);
			po.confused = pi.hasStatus(PokemonGen1::STATUS_CONFUSED);
			for (unsigned j = 0; j < std::size(po.moves) && j < moveSet.size(); j++) {
				po.moves[j].id = moveSet[j].getID();
				po.moves[j].pp = moveSet[j].getPP();
				po.moves[j].maxPp = moveSet[j].getMaxPP();
			}
		}
		return result;
	}

	PokemonGen1::BattleAction toGen1(BattleAction ac)
	{
		switch (ac) {
		case EmptyAction:
			return PokemonGen1::EmptyAction;
		case Attack1:
			return PokemonGen1::Attack1;
		case Attack2:
			return PokemonGen1::Attack2;
		case Attack3:
			return PokemonGen1::Attack3;
		case Attack4:
			return PokemonGen1::Attack4;
		case Switch1:
			return PokemonGen1::Switch1;
		case Switch2:
			return PokemonGen1::Switch2;
		case Switch3:
			return PokemonGen1::Switch3;
		case Switch4:
			return PokemonGen1::Switch4;
		case Switch5:
			return PokemonGen1::Switch5;
		case Switch6:
			return PokemonGen1::Switch6;
		case NoAction:
			return PokemonGen1::NoAction;
		case StruggleMove:
			return PokemonGen1::StruggleMove;
		default:
			return PokemonGen1::Run;
		}
	}
}