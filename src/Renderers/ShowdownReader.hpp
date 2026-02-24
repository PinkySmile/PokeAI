//
// Created by PinkySmile on 22/02/2026.
//

#ifndef POKEAI_SHOWDOWNREADER_HPP
#define POKEAI_SHOWDOWNREADER_HPP


#include <deque>
#include <string>
#include <memory>
#include "IResultBasedGenerator.hpp"

class ShowdownReader {
public:
	ShowdownReader(PkmnRenderer::GameState &state);
	void feed(std::vector<PkmnCommon::Event> &output, const std::string &line);

private:
	void _processChunk(std::vector<PkmnCommon::Event> &output);

	struct PState : public IResultBasedGenerator::PState {
		std::vector<std::string> allocatedNames;
	};

	struct MoveContext {
		unsigned lastMoveId = 0;
		bool attackerIsP1 = false;
		bool hasCrit = false;
		bool isSuperEffective = false;
		bool isNotVeryEffective = false;
		bool firstHitEmitted = false;
	};

	std::pair<PState, PState> _pstate;
	std::deque<std::pair<PkmnCommon::Event, IResultBasedGenerator::State>> _usedEvents;
	std::deque<std::pair<PkmnCommon::Event, IResultBasedGenerator::State>> _events;
	std::vector<std::string> _chunk;
	PkmnRenderer::GameState &_state;
	std::unique_ptr<IResultBasedGenerator> _converter;
	MoveContext _moveCtx;
	bool _gameEnded = false;
};


#endif //POKEAI_SHOWDOWNREADER_HPP
