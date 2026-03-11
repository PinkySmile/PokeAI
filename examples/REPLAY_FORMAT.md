# Pokémon Showdown Replay Log Format

This document describes the replay log format used by [Pokémon Showdown](https://pokemonshowdown.com/), a popular online Pokémon battle simulator. Replays in the `examples/` folder are downloaded from the Showdown replay server and follow this format.

## Overview

- **Protocol**: Newline-and-pipe-delimited text stream
- **Encoding**: UTF-8
- **File extension**: `.log` (when saved from the replay server)
- **Source**: Replays are fetched from `https://replay.pokemonshowdown.com/<ID>.log`

### File formats

Replay data can appear in two forms:

1. **Plain text**: Lines start with `|` and contain the battle log directly.
2. **HTML-embedded**: The log is inside `<script type="text/plain" class="battle-log-data">...</script>`. The content between the tags is the same pipe-delimited format.

## Line structure

Each line is a sequence of fields separated by `|`:

```
|FIELD1|FIELD2|FIELD3|...
```

- The first field is the **message type** (or empty for a spacer).
- A line containing only `|` (or an empty line) acts as a **chunk separator** — it clears the message bar and adds a spacer in the battle history.
- Lines not starting with `|` are typically ignored (e.g. HTML outside the script block).

## Message types

### Battle initialization

#### `|player|PLAYER|USERNAME|AVATAR|RATING`

Player details.

| Field    | Description                                                                 |
|----------|-----------------------------------------------------------------------------|
| `PLAYER` | `p1`, `p2`, or `p3`/`p4` in 4-player battles                               |
| `USERNAME` | Display name (may include `☆` prefix for guests)                          |
| `AVATAR` | Avatar identifier (number or custom string like `lucas`)                  |
| `RATING` | Elo rating in the format; blank if unrated                                 |

**Examples**:
- `|player|p1|TR0LLSTER|2|1310` — player with numeric avatar
- `|player|p1|Sasuuna|lucas|1804` — player with named avatar
- `|player|p2|` — player disconnected (empty username)

#### `|teamsize|PLAYER|NUMBER`

Number of Pokémon each player has.

**Example**: `|teamsize|p1|6`

#### `|gametype|GAMETYPE`

Battle format. Values: `singles`, `doubles`, `triples`, `multi`, `freeforall`.

**Example**: `|gametype|singles`

#### `|gen|GENNUM`

Generation number (1–9). Stadium/Let's Go count as their respective gens.

**Example**: `|gen|1` or `|gen|9`

#### `|tier|FORMATNAME`

Display name of the format.

**Example**: `|tier|[Gen 1] OU` or `|tier|[Gen 9] OU`

#### `|rated` or `|rated|MESSAGE`

Indicates a rated game (affects Elo) or another official context (e.g. tournament).

#### `|rule|RULE: DESCRIPTION`

One line per rule/clause. Appears multiple times.

**Examples**:
- `|rule|Species Clause: Limit one of each Pokémon`
- `|rule|HP Percentage Mod: HP is shown in percentages`

#### `|clearpoke`

Marks the start of Team Preview (formats that use it).

#### `|poke|PLAYER|DETAILS|ITEM`

Declares a Pokémon in Team Preview.

| Field    | Description                                                                 |
|----------|-----------------------------------------------------------------------------|
| `PLAYER` | `p1` or `p2`                                                               |
| `DETAILS`| Species and optional modifiers (see [Identifying Pokémon](#identifying-pokémon)) |
| `ITEM`   | `item` if holding an item, otherwise blank                                |

**Example**: `|poke|p1|Samurott-Hisui, M|`

#### `|teampreview`

End of Team Preview phase.

#### `|start`

Battle has started; first switches and moves follow.

---

### Battle progress

#### `|t:|TIMESTAMP`

UNIX timestamp (seconds since 1970) for when events occurred.

**Example**: `|t:|1771129353`

#### `|turn|NUMBER`

Current turn number.

**Example**: `|turn|1`

#### `|upkeep`

Upkeep phase; field conditions (weather, terrain, etc.) are updated.

#### `|inactive|MESSAGE` / `|inactiveoff|MESSAGE`

Timer-related message (e.g. "Battle timer is ON", "Player has X seconds left"). Shown in red in the client.

---

### Major actions

#### `|switch|POKEMON|DETAILS|HP STATUS`

A Pokémon has switched in (voluntary).

| Field    | Description                                                                 |
|----------|-----------------------------------------------------------------------------|
| `POKEMON`| Slot identifier, e.g. `p1a: Jynx` (see [Identifying Pokémon](#identifying-pokémon)) |
| `DETAILS`| Species and optional level/gender/shiny (e.g. `Jynx`, `Primarina, F, shiny`)  |
| `HP STATUS` | Current HP and status (see [HP format](#hp-and-status-format))          |

**Example**: `|switch|p1a: Jynx|Jynx|100/100`

#### `|drag|POKEMON|DETAILS|HP STATUS`

Same as `|switch|` but for forced switches (Whirlwind, Roar, Dragon Tail, etc.).

**Example**: `|drag|p1a: Landorus|Landorus-Therian, M, shiny|100/100`

#### `|move|POKEMON|MOVE|TARGET`

A move was used.

| Field   | Description                                                                 |
|---------|-----------------------------------------------------------------------------|
| `POKEMON` | User (e.g. `p1a: Exeggutor`)                                              |
| `MOVE`  | Move name                                                                   |
| `TARGET`| Target (e.g. `p2a: Rhydon`); may be omitted for multi-target or no-target moves |

Optional suffixes on the line:
- `|[miss]` — move missed
- `|[still]` — no animation
- `|[from] EFFECT` — continuation of an effect (e.g. trapping)
- `|[from] move: X` — used via Metronome/Sleep Talk/Mirror Move
- `|[anim] MOVE2` — use animation of `MOVE2` instead (e.g. Tera Blast)

**Example**: `|move|p1a: Exeggutor|Mega Drain|p2a: Rhydon`

#### `|cant|POKEMON|REASON` or `|cant|POKEMON|REASON|MOVE`

The Pokémon could not act.

**Reasons**: `slp`, `frz`, `par`, `recharge`, `flinch`, `disable`, etc.

**Example**: `|cant|p2a: Rhydon|par`

#### `|faint|POKEMON`

The Pokémon has fainted.

**Example**: `|faint|p2a: Chansey`

---

### Minor actions (effects)

These describe what happens as a result of moves and abilities.

#### `|-damage|POKEMON|HP STATUS`

Damage taken. Optional `|[from] SOURCE` indicates cause (e.g. `psn`, `tox`, `brn`, `confusion`, `drain`, `item: Rocky Helmet`).

**Examples**:
- `|-damage|p2a: Rhydon|8/100 par`
- `|-damage|p2a: Chansey|0 fnt`
- `|-damage|p2a: Cinderace|47/100|[from] item: Rocky Helmet|[of] p1a: Pecharunt`

#### `|-heal|POKEMON|HP STATUS`

Healing. Optional `|[from] SOURCE` (e.g. `drain`, `item: Leftovers`, `move: Wish`). `|[silent]` suppresses the message.

**Example**: `|-heal|p1a: Exeggutor|89/100 par|[from] drain|[of] p2a: Chansey`

#### `|-status|POKEMON|STATUS`

Status inflicted. Optional `|[from] move: X` or `|[silent]`.

**Statuses**: `slp`, `par`, `brn`, `psn`, `tox`, `frz`, `fnt`

**Example**: `|-status|p1a: Exeggutor|par`

#### `|-curestatus|POKEMON|STATUS`

Status cured.

**Example**: `|-curestatus|p1a: Jynx|slp`

#### `|-boost|POKEMON|STAT|AMOUNT`

Stat increased. Stats: `atk`, `def`, `spa`, `spd`, `spe`, `accuracy`, `evasion`.

**Example**: `|-boost|p2a: Clefable|spa|1`

#### `|-unboost|POKEMON|STAT|AMOUNT`

Stat decreased.

**Example**: `|-unboost|p2a: Chansey|spa|1`

#### `|-crit|POKEMON`

Critical hit.

**Example**: `|-crit|p2a: Chansey`

#### `|-supereffective|POKEMON`

Move was super effective.

**Example**: `|-supereffective|p2a: Rhydon`

#### `|-resisted|POKEMON`

Move was not very effective.

**Example**: `|-resisted|p1a: Primarina`

#### `|-immune|POKEMON`

Target was immune.

**Example**: `|-immune|p2a: Landorus`

#### `|-miss|SOURCE|TARGET`

Move missed.

**Example**: `|-miss|p1a: Jynx|p2a: Rhydon`

#### `|-fail|POKEMON|ACTION` / `|-block|POKEMON|EFFECT|...`

Move or effect failed or was blocked.

#### `|-start|POKEMON|EFFECT`

Volatile status started (Substitute, confusion, Taunt, etc.).

**Examples**:
- `|-start|p1a: Iron Crown|move: Future Sight`
- `|-start|p2a: Alomomola|confusion|[from] ability: Poison Puppeteer|[of] p1a: Pecharunt`

#### `|-end|POKEMON|EFFECT`

Volatile status ended. `|[silent]` may suppress the message.

**Example**: `|-end|p1a: Iron Crown|Quark Drive|[silent]`

#### `|-activate|POKEMON|EFFECT`

Miscellaneous effect activation (e.g. confusion self-hit, ability trigger).

**Example**: `|-activate|p2a: Alomomola|confusion`

#### `|-ability|POKEMON|ABILITY`

Ability revealed or changed. Optional `|[from] EFFECT` or `|boost` for Intimidate-style drops.

**Example**: `|-ability|p2a: Kyurem|Pressure`

#### `|-enditem|POKEMON|ITEM`

Item lost (Knock Off, consumed berry, etc.). Optional `|[from] move: X|[of] SOURCE`.

**Example**: `|-enditem|p1a: Primarina|Assault Vest|[from] move: Knock Off|[of] p2a: Goodra`

#### `|-hitcount|POKEMON|NUM`

Multi-hit move hit this many times.

**Example**: `|-hitcount|p2a: Cinderace|2`

#### `|-prepare|POKEMON|MOVE` or `|-prepare|POKEMON|MOVE|TARGET`

Charge move (Dig, Fly, Sky Drop, etc.).

**Example**: `|-prepare|p1a: Charizard|Fly|p2a: Venusaur`

#### `|-terastallize|POKEMON|TYPE`

Pokémon Terastallized to the given type (Gen 9).

**Example**: `|-terastallize|p1a: Dragapult|Ghost`

#### `|-transform|POKEMON|SPECIES` / `|-mega|POKEMON|STONE` / `|-burst|POKEMON|SPECIES|ITEM`

Form changes (Transform, Mega Evolution, Ultra Burst).

#### `|-message|MESSAGE`

Custom message (forfeits, mod messages, etc.).

**Example**: `|-message|rowdy_yates forfeited.`

#### `|-nothing`

Move had no effect (deprecated; newer logs may use `-activate` instead).

---

### Battle end

#### `|win|USERNAME`

Winner declared.

**Example**: `|win|TR0LLSTER`

#### `|tie`

Battle ended in a tie.

---

### Chat and presence

#### `|j|USERNAME`

User joined the room.

**Example**: `|j|☆TR0LLSTER`

#### `|l|USERNAME`

User left the room.

**Example**: `|l|☆rowdy_yates`

#### `|c|USERNAME|MESSAGE`

Chat message.

**Example**: `|c|☆Sasuuna|gg`

#### `|raw|HTML`

Raw HTML to display (e.g. rating changes).

**Example**: `|raw|TR0LLSTER's rating: 1310 &rarr; <strong>1329</strong><br />(+19 for winning)`

---

## Identifying Pokémon

### Pokémon ID (`POKEMON`)

Format: `POSITION: NAME`

- **POSITION**: `p1a`, `p2a`, etc.
  - `p1`, `p2` = player
  - `a`, `b`, `c` = slot (singles: `a`; doubles: `a` left, `b` right; triples: `a`, `b`, `c`)
- **NAME**: Nickname or species if no nickname

**Examples**: `p1a: Jynx`, `p2a: Goodra-Hisui`, `p1a: Primarina`

### DETAILS string

Comma-separated: species and optional modifiers.

| Modifier | Meaning                    | Example        |
|----------|----------------------------|----------------|
| `L##`    | Level (omit if 100)         | `L59`          |
| `M`      | Male                       | `M`            |
| `F`      | Female                     | `F`            |
| `shiny`  | Shiny                      | `shiny`        |
| `tera:TYPE` | Terastallized (Gen 9)  | `tera:Ghost`   |

**Examples**:
- `Jynx` — level 100, no gender
- `Primarina, F, shiny` — female, shiny
- `Goodra-Hisui, F` — Hisuian Goodra, female
- `Samurott-Hisui, M, shiny` — Hisuian Samurott, male, shiny

### HP and status format

**HP**: `CURRENT/MAX` or `CURRENT/100` (with HP Percentage Mod)

**Status**: Optional suffix after HP (see [Appendix B: Status codes](#appendix-b-status-codes-non-volatile))

**Examples**:
- `100/100` — full HP
- `78/100 par` — 78% HP, paralyzed
- `0 fnt` — fainted
- `8/100 par` — 8% HP, paralyzed

**Parsing rules**:
- If no `/`: parse integer up to first space (e.g. `0 fnt` → current=0, max=100 assumed)
- If `/` present: `CURRENT` = substring before `/`, `MAX` = substring after `/` up to first space
- Status suffix: everything after the first space in the HP field

---

## Tags on messages

Many minor actions support optional tags:

| Tag            | Meaning                                      |
|----------------|----------------------------------------------|
| `[from] X`     | Cause of the effect (move, ability, item, status) |
| `[of] POKEMON` | Source Pokémon                              |
| `[silent]`     | Suppress message/animation                  |
| `[still]`      | Suppress animation                          |
| `[miss]`       | Move missed                                 |
| `[anim] MOVE`  | Use different move animation                |
| `[wisher] X`   | Pokémon that used Wish (for Wish healing)   |
| `[eat]`        | Item consumed (e.g. berry on `-enditem`)    |
| `[zeffect]`    | Z-move effect (Gen 7)                      |
| `[upkeep]`     | Effect continues from previous turn        |
| `[partiallytrapped]` | Binding/trapping damage              |

**Example**: `|-damage|p2a: Cinderace|47/100|[from] item: Rocky Helmet|[of] p1a: Pecharunt`

---

## Chunk structure

- A **chunk** is a group of related messages (e.g. one turn’s actions).
- Chunks are separated by a line containing only `|` (or an empty line).
- The reader in this project (`ShowdownReader`) buffers lines until it sees `|` alone, then processes the chunk.

---

## Downloading replays

The `dl_replays.sh` script fetches replays via:

1. **Search**: `https://replay.pokemonshowdown.com/search.json?format=FORMAT&before=TIMESTAMP`
2. **Replay**: `https://replay.pokemonshowdown.com/<ID>.log`

Example formats: `gen1ou`, `gen9ou`, `gen9rand`, `gen9dou`, `gen9triples`, etc.

---

## Example walkthrough

A minimal Gen 1 battle start might look like:

```
|player|p1|Alice|1|1200
|player|p2|Bob|2|1150
|gen|1
|gametype|singles
|teamsize|p1|6
|teamsize|p2|6
|start
|switch|p1a: Jynx|Jynx|100/100
|switch|p2a: Starmie|Starmie|100/100
|turn|1
|
|t:|1771129366
|move|p2a: Starmie|Thunder Wave|p1a: Jynx
|-status|p1a: Jynx|par
|
|upkeep
|turn|2
```

Interpretation: Alice and Bob start a Gen 1 singles battle. Both send out Jynx and Starmie. On turn 1, Starmie uses Thunder Wave and paralyzes Jynx. The `|` alone marks the end of the turn’s message chunk.

---

## Appendix A: Complete message type index

Quick reference for all message types. See main sections for field details.

**Battle init**: `player`, `teamsize`, `gametype`, `gen`, `tier`, `rated`, `rule`, `clearpoke`, `poke`, `teampreview`, `start`

**Progress**: ` ` (spacer), `t:`, `turn`, `upkeep`, `inactive`, `inactiveoff`, `request`

**Major**: `switch`, `drag`, `replace`, `move`, `cant`, `faint`, `detailschange`, `-formechange`, `swap`

**Minor**: `-damage`, `-heal`, `-sethp`, `-status`, `-curestatus`, `-cureteam`, `-boost`, `-unboost`, `-setboost`, `-swapboost`, `-invertboost`, `-clearboost`, `-clearallboost`, `-clearpositiveboost`, `-clearnegativeboost`, `-copyboost`, `-crit`, `-supereffective`, `-resisted`, `-immune`, `-miss`, `-fail`, `-block`, `-notarget`, `-start`, `-end`, `-activate`, `-item`, `-enditem`, `-ability`, `-endability`, `-transform`, `-mega`, `-primal`, `-burst`, `-zpower`, `-zbroken`, `-weather`, `-fieldstart`, `-fieldend`, `-sidestart`, `-sideend`, `-swapsideconditions`, `-terastallize`, `-hitcount`, `-prepare`, `-mustrecharge`, `-nothing`, `-message`, `-hint`, `-combine`, `-waiting`, `-singlemove`, `-singleturn`, `-center`

**End**: `win`, `tie`

**Chat**: `j`, `l`, `c`, `n`, `raw`, `html`, `uhtml`, `badge`, `error`

---

## Appendix B: Status codes (non-volatile)

Used in `-status`, `-curestatus`, `-damage`, `-heal`, and HP status suffix.

| Code | Meaning |
|------|---------|
| `slp` | Sleep |
| `par` | Paralysis |
| `brn` | Burn |
| `psn` | Poison |
| `tox` | Badly Poisoned (Toxic) |
| `frz` | Freeze |
| `fnt` | Fainted (used when HP is 0) |
| `cfz` | Confused (rare; confusion is usually a volatile) |

**Note**: `fnt` appears in HP strings like `0 fnt` — do not treat it as a curable status. When HP is 0, `STATUS` may be ignored per SIM-PROTOCOL.

---

## Appendix C: Stat abbreviations

Used in `-boost`, `-unboost`, `-setboost`, `-swapboost`, etc.

| Code | Stat |
|------|------|
| `atk` | Attack |
| `def` | Defense |
| `spa` | Special Attack |
| `spd` | Special Defense |
| `spe` | Speed |
| `accuracy` | Accuracy |
| `evasion` | Evasion |

---

## Appendix D: Cant reasons

Used in `|cant|POKEMON|REASON|MOVE`. The optional fourth field is the move that was blocked.

| Reason | Meaning |
|--------|---------|
| `slp` | Asleep |
| `frz` | Frozen |
| `par` | Paralyzed |
| `recharge` | Must recharge (e.g. after Hyper Beam) |
| `flinch` | Flinched |
| `disable` | Move disabled by Disable |
| `encore` | Encored into another move |
| `healreplacement` | Healing replacement (e.g. Healing Wish) |
| `torment` | Torment prevents repeating last move |
| `taunt` | Taunt prevents status moves |
| `imprison` | Imprison blocks known moves |
| `gravity` | Gravity prevents certain moves |
| `attract` | Attract prevents movement |
| `trapped` | Trapped (e.g. Mean Look) |
| `focuspunch` | Focus Punch interrupted by damage |
| `nopp` | No PP left |

---

## Appendix E: Weather

Used in `|-weather|WEATHER`. Value `none` when weather ends.

| Code | Meaning |
|------|---------|
| `RainDance` | Rain |
| `SunnyDay` | Harsh sunlight |
| `Sandstorm` | Sandstorm |
| `Hail` | Hail (Gen 1–8) |
| `Snowscape` | Snow (Gen 9) |
| `PrimordialSea` | Primordial Sea (Kyogre-Primal) |
| `DesolateLand` | Desolate Land (Groudon-Primal) |
| `DeltaStream` | Delta Stream (Rayquaza-Mega) |

Optional `|[upkeep]` means weather was already active and continues.

---

## Appendix F: Field conditions

Used in `|-fieldstart|CONDITION` and `|-fieldend|CONDITION`.

| Condition | Meaning |
|-----------|---------|
| `Trick Room` | Trick Room |
| `Magic Room` | Magic Room |
| `Wonder Room` | Wonder Room |
| `Gravity` | Gravity |
| `Mud Sport` | Mud Sport |
| `Water Sport` | Water Sport |
| `Grassy Terrain` | Grassy Terrain |
| `Electric Terrain` | Electric Terrain |
| `Psychic Terrain` | Psychic Terrain |
| `Misty Terrain` | Misty Terrain |

---

## Appendix G: Side conditions

Used in `|-sidestart|SIDE|CONDITION` and `|-sideend|SIDE|CONDITION`. `SIDE` is `p1` or `p2`.

| Condition | Meaning |
|-----------|---------|
| `Stealth Rock` | Stealth Rock |
| `Spikes` | Spikes (may have level: `Spikes` or `Spikes2` etc.) |
| `Toxic Spikes` | Toxic Spikes |
| `Sticky Web` | Sticky Web |
| `Reflect` | Reflect |
| `Light Screen` | Light Screen |
| `Aurora Veil` | Aurora Veil |
| `Tailwind` | Tailwind |
| `Safeguard` | Safeguard |
| `Mist` | Mist |
| `Lucky Chant` | Lucky Chant |
| `Wish` | Wish (pending) |
| `Future Sight` | Future Sight (pending) |
| `Doom Desire` | Doom Desire (pending) |

---

## Appendix H: Volatile statuses (EFFECT in -start/-end)

Used in `|-start|POKEMON|EFFECT` and `|-end|POKEMON|EFFECT`. Common values:

| Effect | Meaning |
|--------|---------|
| `Substitute` | Substitute |
| `confusion` | Confusion |
| `typechange` | Type changed (e.g. Conversion) |
| `Leech Seed` | Leech Seed |
| `Curse` | Curse (Ghost) |
| `Taunt` | Taunt |
| `Encore` | Encore |
| `Torment` | Torment |
| `Disable` | Disable |
| `Imprison` | Imprison |
| `Heal Block` | Heal Block |
| `Embargo` | Embargo |
| `Magnet Rise` | Magnet Rise |
| `Telekinesis` | Telekinesis |
| `Aqua Ring` | Aqua Ring |
| `Ingrain` | Ingrain |
| `move: X` | Move-specific (e.g. `move: Future Sight`, `move: Bide`) |
| `ability: X` | Ability-specific (e.g. `ability: Poison Puppeteer`) |
| `item: X` | Item-specific |

---

## Appendix I: Damage/heal [from] sources

Common values for `|[from] SOURCE` on `-damage` and `-heal`:

| Source | Meaning |
|--------|---------|
| `psn` | Poison damage |
| `tox` | Toxic damage |
| `brn` | Burn damage |
| `confusion` | Confusion self-hit |
| `drain` | Drain move (e.g. Mega Drain) |
| `recoil` | Recoil damage |
| `item: X` | Item (e.g. `item: Rocky Helmet`, `item: Leftovers`) |
| `move: X` | Move (e.g. `move: Wish`, `move: Leech Seed`) |
| `ability: X` | Ability |

---

## Appendix J: Message types to ignore (optional)

These can be safely ignored when building battle state. The client may still display them.

| Type | Reason |
|------|--------|
| `j` | Join — presence only |
| `l` | Leave — presence only |
| `n` | Name change |
| `c` | Chat |
| `t:` | Timestamp — for replay timing |
| `gametype` | Redundant with battle state |
| `rated` | Metadata |
| `tier` | Metadata |
| `upkeep` | Phase marker |
| `inactive` | Timer message |
| `inactiveoff` | Timer message |
| `rule` | Metadata |
| `raw` | Raw HTML display |
| `html` | HTML injection |
| `uhtml` | Update HTML |
| `badge` | User badge |
| `clearpoke` | Team preview start marker |
| `poke` | Team preview (optional to parse) |
| `teampreview` | Team preview end |
| `-hint` | Parenthetical hint |
| `-mustrecharge` | Redundant with `cant` |
| `-transform` | May be handled by `-replace` |

---

## Appendix K: Parsing order and dependencies

1. **Before battle**: Parse `player`, `teamsize`, `gen`, `gametype`, `tier`, `rule`, `clearpoke`, `poke`, `teampreview`, `start`.
2. **Chunk processing**: Buffer lines until `|` alone. Process chunk as a unit.
3. **Turn structure**: `turn` marks turn start; `upkeep` marks upkeep phase.
4. **Switch before move**: `switch`/`drag` sets active Pokémon; `move` uses `POKEMON` to identify user.
5. **Damage context**: `-damage` and `-heal` apply to the Pokémon in field 2; `-crit`, `-supereffective`, `-resisted` often precede `-damage` for the same hit.
6. **Move continuation**: `|move|` with `[from]` but not `[from] move:` indicates a trapping/binding move continuing damage (e.g. Wrap).

---

## Appendix L: Edge cases and special parsing

### Switch DETAILS vs POKEMON name

- `POKEMON` in `|switch|p1a: Goodra|Goodra-Hisui, F|100/100` — the first part (`p1a: Goodra`) uses nickname or short name; the second part (`Goodra-Hisui, F`) is the full DETAILS with species.
- For species with different formes (e.g. `Goodra-Hisui`), DETAILS has the full forme; POKEMON may abbreviate.

### Team Preview DETAILS

- In `|poke|`, DETAILS may omit level and shininess (hidden until battle).
- Forme may be `-*` for unrevealed (e.g. `Arceus-*`).

### HP without slash

- `0 fnt` — current HP 0; max HP typically 100 (percentage mode) or unknown.
- Parser should handle missing `MAX` by defaulting to 100 when HP Percentage Mod is in effect.

### Inactive Pokémon ID

- `p1: Dragonite` (no slot letter) — Pokémon not in active slot (e.g. during Heal Bell).

### Request messages

- `|request|REQUEST` — JSON object for choice. Not present in saved replays (replays are post-battle logs).

### Error messages

- `|error|[Invalid choice] MESSAGE` — sent during live play; not in replays.

### Generation-specific

- Gen 1: No `-fail`; use `-notarget` for some failures. No Terastallization, Dynamax, Z-moves.
- Gen 9: `-terastallize`, `tera:TYPE` in DETAILS, Snowscape instead of Hail.

---

## References

- [Pokémon Showdown SIM-PROTOCOL.md](https://github.com/smogon/pokemon-showdown/blob/master/sim/SIM-PROTOCOL.md) — official simulator protocol
- [@pkmn/protocol](https://www.npmjs.com/@pkmn/protocol) — TypeScript/JavaScript parser for the protocol
