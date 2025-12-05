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
#include <SFML/Window/Event.hpp>
#include "GameEngine/Gen1/State.hpp"
#include "GameEngine/Event.hpp"

class IRenderer {
public:
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
		std::string desc;
	};

	struct Pokemon {
		unsigned id = 0;
		unsigned hp;
		unsigned maxHp;
		unsigned atk;
		unsigned def;
		unsigned spd;
		unsigned spe;
		unsigned level;
		char name[32];
		std::array<Move, 4> moves;
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
		char name[32];
		std::array<Pokemon, 6> team;
		unsigned active;
		unsigned spriteId;
		bool acidArmor;
		bool exploded;
		bool hidden;
		bool substitute;
	};

	struct GameState {
		PlayerState p1;
		PlayerState p2;
	};

	bool soundDisabled = false;
	bool musicDisabled = false;
	bool waiting = false;
	GameState state;

	virtual ~IRenderer() = default;
	virtual void update() = 0;
	virtual void render(sf::RenderTarget &) = 0;
	virtual sf::Vector2u getSize() const = 0;
	virtual void reset() = 0;
	virtual void consumeEvent(const PkmnCommon::Event &event);
	virtual void consumeEvent(const sf::Event &event) = 0;
	virtual std::optional<BattleAction> selectAction(bool attackDisabled) = 0;
	virtual const sf::Texture &getPkmnFace(unsigned pkmnId) = 0;
	virtual const sf::SoundBuffer &getPkmnCry(unsigned int pkmnId) = 0;
	virtual void previousTurn();
	virtual void nextTurn();
	void goToTurn(unsigned turn);
	unsigned getTurn();

protected:
	struct SavedState {
		GameState state;
		std::deque<PkmnCommon::Event> queue;
		unsigned turn;
	};

	bool _skipping = false;
	std::vector<SavedState> _snapshots;
	std::deque<PkmnCommon::Event> _queue;
	unsigned _currentTurn = 0;
};

IRenderer::GameState fromGen1(const PokemonGen1::BattleState &state);
PokemonGen1::BattleAction toGen1(IRenderer::BattleAction);


#endif //POKEAI_IRENDERER_HPP
