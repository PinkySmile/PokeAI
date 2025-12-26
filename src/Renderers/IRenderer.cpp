//
// Created by PinkySmile on 19/11/2025.
//

#include <cstring>
#include "IRenderer.hpp"
#include <SFML/Graphics/Image.hpp>

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

	void IRenderer::consumeEvent(const PkmnCommon::Event &event)
	{
		this->_queue.push_back(event);
		for (auto &s : this->_snapshots)
			s.queue.push_back(event);
	}

	GameState fromGen1(const PokemonGen1::BattleState &state)
	{
		GameState result;

		strcpy(result.p1.name, state.me.name.c_str());
		result.p1.substitute = state.me.team[state.me.pokemonOnField].hasSubstitute();
		result.p1.active = state.me.pokemonOnField;
		result.p1.spriteId = state.me.team[state.me.pokemonOnField].getID();
		result.p1.hidden = false;
		result.p1.acidArmor = false;
		result.p1.exploded = false;
		for (unsigned i = 0; i < result.p1.team.size() && i < state.me.team.size(); i++) {
			auto &po = result.p1.team[i];
			auto &pi = state.me.team[i];
			auto &moveSet = pi.getMoveSet();

			po.id = pi.getID();
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
			for (unsigned j = 0; j < po.moves.size() && j < moveSet.size(); j++) {
				po.moves[j].id = moveSet[j].getID();
				po.moves[j].pp = moveSet[j].getPP();
				po.moves[j].maxPp = moveSet[j].getMaxPP();
			}
		}

		strcpy(result.p2.name, state.op.name.c_str());
		result.p2.substitute = state.op.team[state.op.pokemonOnField].hasSubstitute();
		result.p2.active = state.op.pokemonOnField;
		result.p2.spriteId = state.op.team[state.me.pokemonOnField].getID();
		result.p2.hidden = false;
		result.p2.acidArmor = false;
		result.p2.exploded = false;
		for (unsigned i = 0; i < result.p2.team.size() && i < state.op.team.size(); i++) {
			auto &po = result.p2.team[i];
			auto &pi = state.op.team[i];
			auto &moveSet = pi.getMoveSet();

			po.id = pi.getID();
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
			for (unsigned j = 0; j < po.moves.size() && j < moveSet.size(); j++) {
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