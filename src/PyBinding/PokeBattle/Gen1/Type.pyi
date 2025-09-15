class Type(int):
	Normal: Type
	Fighting: Type
	Flying: Type
	Poison: Type
	Ground: Type
	Rock: Type
	Invalid: Type
	Bug: Type
	Ghost: Type
	Fire: Type
	Water: Type
	Grass: Type
	Electric: Type
	Psychic: Type
	Ice: Type
	Dragon: Type
	Neutral_Physical: Type
	Neutral_Special: Type

	@property
	def name(self) -> str: ...


def type_to_string(type: int) -> str: ...
def type_to_string_short(type: int) -> str: ...
def get_attack_damage_multiplier(attack: int, defender: int|tuple[int, int]) -> float: ...