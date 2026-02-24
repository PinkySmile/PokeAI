//
// Created by PinkySmile on 22/02/2026.
//

#ifndef POKEAI_GEN1RESULTBASEDGENERATOR_HPP
#define POKEAI_GEN1RESULTBASEDGENERATOR_HPP


#include "IResultBasedGenerator.hpp"

class Gen1ResultBasedGenerator : public IResultBasedGenerator {
public:
	bool convertEvent(
		const PkmnRenderer::GameState &state,
		std::deque<std::pair<PkmnCommon::Event, State>> &usedEvents,
		std::deque<std::pair<PkmnCommon::Event, State>> &events,
		std::vector<PkmnCommon::Event> &output
	) override;
};


#endif //POKEAI_GEN1RESULTBASEDGENERATOR_HPP