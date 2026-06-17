//
// Created by PinkySmile on 19/11/2025.
//

#ifndef POKEAI_IRENDERER_HPP
#define POKEAI_IRENDERER_HPP


#include <variant>
#include <string>
#include <vector>
#include <deque>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Window/Event.hpp>
#include "GameEngine/Gen1/State.hpp"
#include "GameEngine/Event.hpp"

namespace PkmnRenderer
{
	enum BattleAction : unsigned char {
		EmptyAction,
		Attack1,
		Attack2,
		Attack3,
		Attack4,
		Switch1,
		Switch2,
		Switch3,
		Switch4,
		Switch5,
		Switch6,
		NoAction,
		StruggleMove,
		Run,
	};

	struct Move {
		unsigned id;
		unsigned pp;
		unsigned maxPp = 0;
		unsigned power;
		unsigned acc;
		char desc[256];
	};

	struct Pokemon {
		PkmnCommon::PokemonSpecies id = PkmnCommon::Missingno;
		unsigned hp;
		unsigned maxHp;
		unsigned atk;
		unsigned def;
		unsigned spd;
		unsigned spe;
		unsigned level;
		Move moves[4];
		char name[33];
		bool asleep;
		bool frozen;
		bool burned;
		bool poisoned;
		bool toxicPoisoned;
		bool paralyzed;
		bool ko;
		bool leeched;
		bool confused;
	};

	struct PlayerState {
		Pokemon team[6];
		unsigned active = 0;
		PkmnCommon::PokemonSpecies spriteId;
		char name[33];
		bool acidArmor;
		bool exploded;
		bool hidden;
		bool substitute;
	};

	struct GameState {
		PlayerState p1;
		PlayerState p2;
	};

	class IRenderer {
	public:
		bool displayTurn = true;
		bool soundDisabled = false;
		bool musicDisabled = false;
		bool waiting = false;
		bool swapSide = false;
		GameState state;

		virtual ~IRenderer() = default;
		virtual void update() = 0;
		virtual void render(sf::RenderTarget &) = 0;
		virtual sf::Vector2u getSize() const = 0;
		virtual void reset() = 0;
		virtual void clear();
		virtual void consumeEvent(const PkmnCommon::Event &event);
		virtual void consumeEvent(const sf::Event &event) = 0;
		virtual std::optional<BattleAction> selectAction(sf::RenderTarget &target, bool attackDisabled) = 0;
		virtual const sf::Texture &getPkmnFace(PkmnCommon::PokemonSpecies pkmnId) = 0;
		virtual const sf::SoundBuffer &getPkmnCry(PkmnCommon::PokemonSpecies pkmnId) = 0;
		virtual void previousTurn();
		virtual void nextTurn();
		void goToTurn(unsigned turn);
		unsigned getTurn() const;
		std::vector<unsigned char> renderVec();
		size_t renderBuff(unsigned char *buffer);
		bool hasAnimationEnded() const;
		PlayerState &getState(bool player);

	protected:
		struct SavedState {
			GameState state;
			std::deque<PkmnCommon::Event> queue;
			unsigned turn;
		};

		sf::RenderTexture _buffer;
		bool _skipping = false;
		bool _finished = false;
		std::vector<SavedState> _snapshots;
		std::deque<PkmnCommon::Event> _queue;
		unsigned _currentTurn = 0;
	};

	PkmnCommon::PokemonSpecies gen1SpeciesToCommon(unsigned id);
	GameState fromGen1(const PokemonGen1::BattleState &state);
	PokemonGen1::BattleAction toGen1(BattleAction);
}


#endif //POKEAI_IRENDERER_HPP
