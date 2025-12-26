# cython: language_level=3

from libcpp.string cimport string
from libcpp.vector cimport vector

from ._Event cimport Event
from ._State cimport BattleAction, BattleState

cdef extern from "<Renderers/Gen1Renderer.hpp>" namespace PkmnRenderer:
	ctypedef enum BattleAction:
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
		Run

	struct Move:
		unsigned id
		unsigned pp
		unsigned maxPp
		unsigned power
		unsigned acc
		char desc[256]

	struct Pokemon:
		unsigned id
		unsigned hp
		unsigned maxHp
		unsigned atk
		unsigned _def 'def'
		unsigned spd
		unsigned spe
		unsigned level
		char name[32]
		MoveArray moves
		bool asleep
		bool frozen
		bool burned
		bool poisoned
		bool toxicPoisoned
		bool paralyzed
		bool ko
		bool leeched
		bool confused

	struct PlayerState:
		char name[32]
		PkmnArray team;
		unsigned active
		unsigned spriteId
		bool acidArmor
		bool exploded
		bool hidden
		bool substitute

	struct GameState:
		PlayerState p1
		PlayerState p2


cdef extern from "<array>" namespace "std" nogil:
	cdef cppclass MoveArray "array<PkmnRenderer::Move, 4>":
		MoveArray() except+
		Move &operator[](size_t)

	cdef cppclass PkmnArray "array<PkmnRenderer::Pokemon, 6>":
		PkmnArray() except+
		Pokemon &operator[](size_t)


cdef extern from "<SFML/System/Vector2.hpp>" namespace "sf" nogil:
	cdef cppclass Vector2u:
		unsigned x
		unsigned y
		Vector2u(unsigned, unsigned)


cdef extern from "<Renderers/Gen1Renderer.hpp>" namespace PkmnRenderer:
	cdef cppclass IRenderer:
		bool soundDisabled
		bool musicDisabled
		bool waiting
		GameState state

		void consumeEvent(const Event &event)
		void previousTurn()
		void nextTurn()
		void goToTurn(unsigned turn)
		unsigned getTurn() const
		vector[unsigned char] renderVec()
		size_t renderBuff(unsigned char *buffer)
		bool hasAnimationEnded() const


	GameState fromGen1(const BattleState &state);
	BattleAction toGen1(BattleAction);

