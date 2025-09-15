class Gen1ConnectionStage(int):
	PkmnCenter: Gen1ConnectionStage
	PingingOpponent: Gen1ConnectionStage
	RoomChoose: Gen1ConnectionStage
	PingPokemonExchange: Gen1ConnectionStage
	ExchangePokemons: Gen1ConnectionStage
	Battle: Gen1ConnectionStage


class EmulatorGameHandle:
	def __init__(self): ...

	@property
	def ready(self) -> bool: ...
	@ready.setter
	def ready(self, ready: bool): ...

	@property
	def stage(self) -> Gen1ConnectionStage: ...
	@stage.setter
	def stage(self, stage: Gen1ConnectionStage|int): ...

	@property
	def connected(self) -> bool: ...

	@property
	def battle_sending_progress(self) -> tuple[int, int]: ...

	@staticmethod
	def convert_string(data: str|bytes) -> str|bytes: ...
