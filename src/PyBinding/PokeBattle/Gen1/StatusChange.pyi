class StatusChange(int):
	OK: StatusChange
	Asleep_for_1_turn: StatusChange
	Asleep_for_2_turn: StatusChange
	Asleep_for_3_turn: StatusChange
	Asleep_for_4_turn: StatusChange
	Asleep_for_5_turn: StatusChange
	Asleep_for_6_turn: StatusChange
	Asleep_for_7_turn: StatusChange
	Asleep: StatusChange
	Poisoned: StatusChange
	Burned: StatusChange
	Frozen: StatusChange
	Paralyzed: StatusChange
	KO: StatusChange
	Badly_poisoned: StatusChange
	Any_non_volatile_status: StatusChange
	Leeched: StatusChange
	Confused_for_1_turn: StatusChange
	Confused_for_2_turn: StatusChange
	Confused_for_3_turn: StatusChange
	Confused_for_4_turn: StatusChange
	Confused: StatusChange
	Flinched: StatusChange

	@property
	def name(self) -> str: ...


def status_to_string(s: int) -> str: ...
def status_to_string_short(s: int) -> str: ...