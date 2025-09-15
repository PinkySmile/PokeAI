class StatsChange(int):
	Atk: StatsChange
	Def: StatsChange
	Spd: StatsChange
	Spe: StatsChange
	Evd: StatsChange
	Acc: StatsChange

	@property
	def name(self) -> str: ...


def stat_to_string(stat: int) -> str: ...
def stat_to_little_string(stat: int) -> str: ...