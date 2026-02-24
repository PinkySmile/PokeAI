//
// Created by PinkySmile on 22/02/2026.
//

#ifndef POKEAI_IRESULTBASEDGENERATOR_HPP
#define POKEAI_IRESULTBASEDGENERATOR_HPP


#include <deque>
#include <array>
#include <vector>
#include "IRenderer.hpp"
#include "GameEngine/Event.hpp"

class IResultBasedGenerator {
public:
	struct PState {
		unsigned onField;
		unsigned hp;
		unsigned status;
	};

	typedef std::pair<PState, PState> State;

	virtual ~IResultBasedGenerator() = default;
	virtual bool convertEvent(
		const PkmnRenderer::GameState &state,
		std::deque<std::pair<PkmnCommon::Event, State>> &usedEvents,
		std::deque<std::pair<PkmnCommon::Event, State>> &events,
		std::vector<PkmnCommon::Event> &output
	) = 0;
};


#endif //POKEAI_IRESULTBASEDGENERATOR_HPP