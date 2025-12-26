# cython: language_level=3

from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp cimport bool

from ._Event cimport Event
from ._IRenderer cimport GameState

cdef extern from "<Renderers/Gen1Renderer.hpp>":
	# TODO: Check how to implement inheritance proper
	cdef cppclass Gen1Renderer:
		bool soundDisabled
		bool musicDisabled
		bool waiting
		GameState state

		Gen1Renderer(const string &variant, bool hasColors) except+
		void update()
		void reset()
		void consumeEvent(const Event &event)
		void previousTurn()
		void nextTurn()
		void goToTurn(unsigned turn)
		unsigned getTurn() const
		vector[unsigned char] renderVec()
		size_t renderBuff(unsigned char *buffer)
		bool hasAnimationEnded() const
