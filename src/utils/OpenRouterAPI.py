import os
import json
import requests
from PokeBattle.Gen1.State import BattleState, BattleAction
from PokeBattle.Gen1.StatusChange import status_to_string_short
from PokeBattle.Gen1.Type import type_to_string_short

key = os.getenv("OPEN_ROUTER_KEY")

SYSTEM_PROMPT = """You are an expert pokémon Generation 1 player.
You will be sent pokémon game state, and you must reply with the action you will take.
All the valid actions are: Attack1, Attack2, Attack3, Attack4, Switch1, Switch2, Switch3, Switch4, Switch5, Switch6, StruggleMove.
You must reply with only actions from this list and this list only.
No explanations are needed, only the word from the list.

Attack1 corresponds to the 1st attack from your active pokémon.
Attack2 corresponds to the 2nd attack from your active pokémon.
Attack3 corresponds to the 3rd attack from your active pokémon.
Attack4 corresponds to the 4th attack from your active pokémon.
Switch1 corresponds to switching to the 1st pokémon of your team.
Switch2 corresponds to switching to the 2nd pokémon of your team.
Switch3 corresponds to switching to the 3rd pokémon of your team.
Switch4 corresponds to switching to the 4th pokémon of your team.
Switch5 corresponds to switching to the 5th pokémon of your team.
Switch6 corresponds to switching to the 6th pokémon of your team.
StruggleMove corresponds to using Struggle, when you don't have enough PP.

Hidden pokémons are marked as <Pokémon not revealed yet>.
Moves not revealed are marked "??? ??/??PP".
The active pokémon on field have the "(Active)" mention at the end of the line.

After are included the battle logs from the last turn.

For example:
```
Your team:
 -    PIKACHU (   PIKACHU) l 11, Elc    ,  38/ 38HP,  27ATK ( 27@+0),  21DEF ( 21@+0),  26SPE ( 26@+0),  35SPD ( 35@+0), 100%ACC (+0), 100%EVD (+0), Status: 0x0000     OK, Moves: ThunderShock 30/30PP, Quick Attack 30/30PP,      Agility 20/30PP,        Pound 35/35PP (Active)
 -     MANKEY (    MANKEY) l 10, Fgt    ,  37/ 37HP,  30ATK ( 30@+0),  21DEF ( 21@+0),  21SPE ( 21@+0),  28SPD ( 28@+0), 100%ACC (+0), 100%EVD (+0), Status: 0x0000     OK, Moves:      Scratch 35/35PP,         Leer 30/30PP
 -   NIDORAN~ (  NIDORAN~) l 10, Psn    ,  38/ 38HP,  25ATK ( 25@+0),  22DEF ( 22@+0),  22SPE ( 22@+0),  24SPD ( 24@+0), 100%ACC (+0), 100%EVD (+0), Status: 0x0000     OK, Moves:       Tackle 35/35PP,         Leer 30/30PP,  Horn Attack 25/25PP
 -     PIDGEY (    PIDGEY) l 10, Nor/Fly,  37/ 37HP,  23ATK ( 23@+0),  22DEF ( 22@+0),  21SPE ( 21@+0),  25SPD ( 25@+0), 100%ACC (+0), 100%EVD (+0), Status: 0x0000     OK, Moves:         Gust 35/35PP,  Sand-Attack 15/15PP
Opponent team:
 -    GEODUDE (   GEODUDE) l 10, Roc/Gnd,  37/ 37HP,  30ATK ( 30@+0),  17DEF ( 34@-2),  20SPE ( 20@+0),  18SPD ( 18@+0), 100%ACC (+0), 100%EVD (+0), Status: 0x0000     OK, Moves:       Tackle 35/35PP,          ??? ??/??PP,          ??? ??/??PP,          ??? ??/??PP
 - <Pokémon not revealed yet>
 -       ONIX (      ONIX) l 12, Roc/Gnd,  41/ 41HP,  26ATK ( 26@+0),  54DEF ( 54@+0),  23SPE ( 23@+0),  32SPD ( 32@+0), 100%ACC (+0), 100%EVD (+0), Status: 0x0000     OK, Moves:         Bide 10/10PP,          ??? ??/??PP,      Screech 40/40PP,       Tackle 35/35PP (Active)
 - <Pokémon not revealed yet>
 - <Pokémon not revealed yet>
Battle logs:
PIKACHU used TAIL WHIP!
GEODUDE's DEFENSE fell!
GEODUDE used TACKLE!
```"""

msgs = [
	{
		"role": "developer",
		"content": SYSTEM_PROMPT
	}
]

def get_move(state: BattleState, model, messages):
	data = f"Your team (P1):\n"
	for i, pkmn in enumerate(state.me.team):
		data += f" - {pkmn.dump()}"
		if i == state.me.pokemon_on_field_index:
			data += " (Active)"
		data += "\n"

	data += f"Opponent team (P2):\n"
	for i, pkmn in enumerate(state.op.team):
		data += " - "
		if not state.me.is_pkmn_discovered(i):
			data += "<Pokémon not revealed yet>\n"
			continue
		data += f"{pkmn.get_name(False): >10} ({pkmn.species_name: >10}) l{pkmn.level: >3d}, {type_to_string_short(pkmn.types[0])}"
		if pkmn.types[0] != pkmn.types[1]:
			data += f"/{type_to_string_short(pkmn.types[1])}"
		else:
			data += "    "
		data += f", {pkmn.health: >3d}/{pkmn.max_health: >3d}HP"
		data += f", {pkmn.attack: >3d}ATK ({pkmn.raw_attack: >3d}@{pkmn.stats_upgrade_stages['ATK']:+d})"
		data += f", {pkmn.defense: >3d}DEF ({pkmn.raw_defense: >3d}@{pkmn.stats_upgrade_stages['DEF']:+d})"
		data += f", {pkmn.special: >3d}SPE ({pkmn.raw_special: >3d}@{pkmn.stats_upgrade_stages['SPE']:+d})"
		data += f", {pkmn.speed: >3d}SPD ({pkmn.raw_speed: >3d}@{pkmn.stats_upgrade_stages['SPD']:+d})"
		data += f", {int(pkmn.accuracy_mul * 100): >3d}%ACC ({pkmn.stats_upgrade_stages['ACC']:+d})"
		data += f", {int(pkmn.evasion_mul * 100): >3d}%ACC ({pkmn.stats_upgrade_stages['EVD']:+d})"
		data += f", Status: 0x{pkmn.status:04X} {status_to_string_short(pkmn.status): >6}, "
		if pkmn.substitute is not None:
			data += f"Sub {pkmn.substitute}, "
		data += "Moves: "
		for j in range(4):
			if j:
				data += ", "
			move = pkmn.move_set[j]
			if state.me.is_pkmn_move_discovered(i, j):
				data += f"{move.name: >12}  {move.pp: >2d}/{move.max_pp: >2d}PP"
			else:
				data += "         ??? ??/??PP"
		if i == state.op.pokemon_on_field_index:
			data += " (Active)"
		data += "\n"
	data += "Battle logs:\n"
	data += "\n".join(messages)
	msgs.append({
		"role": "user",
		"content": data
	})

	response = requests.post(
		url="https://openrouter.ai/api/v1/chat/completions",
		headers={
			"Authorization": f"Bearer {key}",
			"Content-Type": "application/json"
		},
		data=json.dumps({
			"model": model,
			"messages": msgs
		})
	)
	j = response.json()
	if 'error' in j:
		raise RuntimeError(j['error']['message'])
	msgs.append(j["choices"][0]["message"])
	action = j["choices"][0]["message"]["content"]
	return getattr(BattleAction, action)


def get_move_manual(state: BattleState, messages):
	data = f"Your team (P1):\n"
	for i, pkmn in enumerate(state.me.team):
		data += f" - {pkmn.dump()}"
		if i == state.me.pokemon_on_field_index:
			data += " (Active)"
		data += "\n"

	data += f"Opponent team (P2):\n"
	for i, pkmn in enumerate(state.op.team):
		data += " - "
		if not state.me.is_pkmn_discovered(i):
			data += "<Pokémon not revealed yet>\n"
			continue
		data += f"{pkmn.get_name(False): >10} ({pkmn.species_name: >10}) l{pkmn.level: >3d}, {type_to_string_short(pkmn.types[0])}"
		if pkmn.types[0] != pkmn.types[1]:
			data += f"/{type_to_string_short(pkmn.types[1])}"
		else:
			data += "    "
		data += f", {pkmn.health: >3d}/{pkmn.max_health: >3d}HP"
		data += f", {pkmn.attack: >3d}ATK ({pkmn.raw_attack: >3d}@{pkmn.stats_upgrade_stages['ATK']:+d})"
		data += f", {pkmn.defense: >3d}DEF ({pkmn.raw_defense: >3d}@{pkmn.stats_upgrade_stages['DEF']:+d})"
		data += f", {pkmn.special: >3d}SPE ({pkmn.raw_special: >3d}@{pkmn.stats_upgrade_stages['SPE']:+d})"
		data += f", {pkmn.speed: >3d}SPD ({pkmn.raw_speed: >3d}@{pkmn.stats_upgrade_stages['SPD']:+d})"
		data += f", {int(pkmn.accuracy_mul * 100): >3d}%ACC ({pkmn.stats_upgrade_stages['ACC']:+d})"
		data += f", {int(pkmn.evasion_mul * 100): >3d}%ACC ({pkmn.stats_upgrade_stages['EVD']:+d})"
		data += f", Status: 0x{pkmn.status:04X} {status_to_string_short(pkmn.status): >6}, "
		if pkmn.substitute is not None:
			data += f"Sub {pkmn.substitute}, "
		data += "Moves: "
		for j in range(4):
			if j:
				data += ", "
			move = pkmn.move_set[j]
			if state.me.is_pkmn_move_discovered(i, j):
				data += f"{move.name: >12}  {move.pp: >2d}/{move.max_pp: >2d}PP"
			else:
				data += "         ??? ??/??PP"
		if i == state.op.pokemon_on_field_index:
			data += " (Active)"
		data += "\n"
	data += "Battle logs:\n"
	data += "\n".join(messages)
	print("-----------------------")
	if len(msgs) == 1:
		print(msgs[0]['content'])
	print(data)

	action = input("Reponse: ")
	msgs.append({
		"role": "user",
		"content": data
	})
	msgs.append({
		'role': 'assistant',
		'content': action
	})
	return getattr(BattleAction, action)