//
// Created by PinkySmile on 19/11/2025.
//

#include <fstream>
#include <nlohmann/json.hpp>
#include "Gen1Renderer.hpp"

#define _isPlayer _gpCounter[0]
#define _animMove _gpCounter[1]
#define _animCounter _gpCounter[2]
#define _subCounter _gpCounter[3]
#define _subSpawnTimer _gpCounter[4]
#define _subSpawnUnspawn _gpCounter[5]
#define _waitCounter _gpCounter[6]
#define _currentCharacter _gpCounter[0]
#define _textTimer _gpCounter[1]
#define _textCounter _gpCounter[2]
#define _currentAnim _gpCounter[4]
#define _notVeryEffective _gpCounter[1]
#define _veryEffective _gpCounter[3]
#define _healthTarget _gpCounter[1]
#define _crySound _gpSound
#define _moveSound _gpSound
#define _hitSound _gpSound
#define _faintSound _gpSound

#define TEXT_LINE_TIME 40
#define TEXT_LINE_SCROLL 10
#define TEXT_WAIT_AFTER 90
#define MOVE_WAIT_BEFORE 60

#define WITHDRAW_ANIM_LENGTH 60

enum AnimationType {
	ANIMTYPE_DELAY,
	ANIMTYPE_STAT_LOWER_PLAYER,
	ANIMTYPE_STAT_LOWER_OPPONENT,
	ANIMTYPE_STATUS_SIDE_EFFECT_LOWER_PLAYER,
	ANIMTYPE_STATUS_SIDE_EFFECT_LOWER_OPPONENT,
};

using namespace PkmnCommon;

void Gen1Renderer::_loadMoveFrames(std::vector<MoveAnim> &m, const nlohmann::json &json)
{
	for (auto &janim : json) {
		m.emplace_back();

		auto &anim = m.back();

		anim.duration = janim["duration"];
		anim.tileset = janim["tileset"];
		if (janim.contains("scx")) {
			anim.palB = janim["palB"].get<std::array<unsigned, 4>>();
			anim.pal0 = janim["pal1"].get<std::array<unsigned, 4>>();
			anim.pal1 = janim["pal2"].get<std::array<unsigned, 4>>();
			anim.scx = janim["scx"];
			anim.scy = janim["scy"];
			anim.wx = janim["wx"];
			anim.wy = janim["wy"];
			anim.p1Off = janim["p1"].get<std::optional<std::pair<int, int>>>();
			anim.p2Off = janim["p2"].get<std::optional<std::pair<int, int>>>();
		}
		for (auto &j: janim["sprites"]) {
			anim.sprites.emplace_back();

			auto &sprite = anim.sprites.back();

			sprite.x = j["x"];
			sprite.y = j["y"];
			sprite.id = j["id"];
			sprite.palNum = j["pal_num"];
			sprite.flip = j["flip"].get<std::pair<bool, bool>>();
			sprite.prio = j["prio"];
		}
	}
}

void Gen1Renderer::_loadMoveData(MoveData &data, const std::string &id)
{
	std::ifstream stream{"assets/gen1/moves/anims/move_" + id + ".json"};
	nlohmann::json json;

	stream >> json;

	if (id != "45" && id != "46") // Growl and Roar
		(void)data.sound.loadFromFile("assets/gen1/moves/sounds/move_" + id + ".ogg");

	Gen1Renderer::_loadMoveFrames(data.animP1, json["frames_p1"]);
	Gen1Renderer::_loadMoveFrames(data.animP2, json["frames_p2"]);
}

void Gen1Renderer::_loadPokemonData(PokemonData &data, const std::string &folder, const std::string &variant)
{
	(void)data.front.init("assets/gen1/pokemons/" + folder + "/front" + variant + ".png");
	(void)data.back.init("assets/gen1/pokemons/" + folder + "/back.png");
	(void)data.icon.loadFromFile("assets/gen1/pokemons/" + folder + "/icon.png");
	(void)data.cry.loadFromFile("assets/gen1/pokemons/" + folder + "/cry.ogg");
	(void)data.roar.loadFromFile("assets/gen1/moves/sounds/move_46/" + folder + ".ogg");
	(void)data.growl.loadFromFile("assets/gen1/moves/sounds/move_45/" + folder + ".ogg");
}

Gen1Renderer::Gen1Renderer(const std::string &variant) :
	_font("assets/gen1/font.ttf")
{
	std::string music = "battle0";
	std::ifstream streamLoop{"assets/gen1/sounds/" + music + "_loop.txt"};
	std::ifstream streamList{"assets/gen1/list.json"};
	std::ifstream streamListMove{"assets/gen1/moves/list.json"};
	std::ifstream streamStartAnim{"assets/gen1/start.json"};
	nlohmann::json json;
	std::string line;

	(void)this->_music.openFromFile("assets/gen1/sounds/" + music + ".ogg");
	this->_music.setVolume(100);

	this->_font.setSmooth(false);
	std::getline(streamLoop, line);
	this->_music.setLoopPoints({
		.offset = sf::seconds(std::stof(line)),
		.length = this->_music.getDuration() - sf::seconds(std::stof(line)),
	});
	this->_music.setLooping(true);

	(void)this->_moveTextures[0].init("assets/gen1/moves/tilemap1.png");
	(void)this->_moveTextures[1].init("assets/gen1/moves/tilemap2.png");

	(void)this->_balls[0].loadFromFile("assets/gen1/pokeballs/pkmnOK.png");
	(void)this->_balls[1].loadFromFile("assets/gen1/pokeballs/pkmnNO.png");
	(void)this->_balls[2].loadFromFile("assets/gen1/pokeballs/pkmnFNT.png");
	(void)this->_balls[3].loadFromFile("assets/gen1/pokeballs/pkmnSTATUS.png");

	(void)this->_hitSounds[0].loadFromFile("assets/gen1/sounds/ne_sound.ogg");
	(void)this->_hitSounds[1].loadFromFile("assets/gen1/sounds/hit_sound.ogg");
	(void)this->_hitSounds[2].loadFromFile("assets/gen1/sounds/ve_sound.ogg");
	(void)this->_trainerLand.loadFromFile("assets/gen1/sounds/trainer_land.ogg");
	(void)this->_ballPopSound.loadFromFile("assets/gen1/sounds/ball_pop.ogg");
	(void)this->_menuSelect.loadFromFile("assets/gen1/sounds/menu_select.ogg");
	(void)this->_faint.loadFromFile("assets/gen1/sounds/faint.ogg");

	(void)this->_trainer[0].init("assets/gen1/redb.png");
	(void)this->_trainer[1].init("assets/gen1/red.png");

	(void)this->_choicesHUD.loadFromFile("assets/gen1/choices.png");
	(void)this->_attackHUD.loadFromFile("assets/gen1/attacks_overlay.png");
	(void)this->_waitingHUD.init("assets/gen1/wait_overlay.png");
	(void)this->_hpOverlay.init("assets/gen1/hp_overlay.png");
	(void)this->_levelSprite.init("assets/gen1/level_icon.png");

	(void)this->_arrows[0].loadFromFile("assets/gen1/arrow.png");
	(void)this->_arrows[1].loadFromFile("assets/gen1/selectArrow.png");

	(void)this->_boxes[0].init("assets/gen1/text_box.png");
	(void)this->_boxes[1].init("assets/gen1/VS_box.png");
	(void)this->_boxes[2].init("assets/gen1/pkmns_border.png");
	(void)this->_boxes[3].init("assets/gen1/pkmns_border_player_side.png");

	streamList >> json;
	Gen1Renderer::_loadPokemonData(this->_missingno, "missingno", "");
	for (auto &id : json) {
		unsigned id_ = id;

		Gen1Renderer::_loadPokemonData(this->_data[id_], std::to_string(id_), variant);
	}

	streamListMove >> json;
	for (auto &id : json) {
		unsigned id_ = id;

		Gen1Renderer::_loadMoveData(this->_moveData[id_], std::to_string(id_));
	}

	streamStartAnim >> json;
	Gen1Renderer::_loadMoveFrames(this->_ballPopAnim, json);
}

bool (Gen1Renderer::*Gen1Renderer::_updates[])() = {
	&Gen1Renderer::_updateNormal,
	&Gen1Renderer::_updateGameStart,
	&Gen1Renderer::_updateGameEnd,
	&Gen1Renderer::_updateHit,
	&Gen1Renderer::_updateDeath,
	&Gen1Renderer::_updateSwitch,
	&Gen1Renderer::_updateWithdraw,
	&Gen1Renderer::_updateHealthMod,
	&Gen1Renderer::_updateExtraAnim,
	&Gen1Renderer::_updateAnim,
	&Gen1Renderer::_updateMove,
	&Gen1Renderer::_updateTurnStart,
	&Gen1Renderer::_updateText
};

void (Gen1Renderer::*Gen1Renderer::_renderers[])(sf::RenderTarget &) = {
	&Gen1Renderer::_renderNormal,
	&Gen1Renderer::_renderGameStart,
	&Gen1Renderer::_renderGameEnd,
	&Gen1Renderer::_renderHit,
	&Gen1Renderer::_renderDeath,
	&Gen1Renderer::_renderSwitch,
	&Gen1Renderer::_renderWithdraw,
	&Gen1Renderer::_renderHealthMod,
	&Gen1Renderer::_renderExtraAnim,
	&Gen1Renderer::_renderAnim,
	&Gen1Renderer::_renderMove,
	&Gen1Renderer::_renderTurnStart,
	&Gen1Renderer::_renderText
};

void Gen1Renderer::update()
{
	while (!(this->*Gen1Renderer::_updates[this->_currentEvent])()) {
		this->_currentEvent = EVNTTYPE_NONE;
		if (this->_queue.empty())
			break;
		this->_handleEvent(this->_queue.front());
		this->_queue.pop_front();
	}
}

void Gen1Renderer::render(sf::RenderTarget &target)
{
	(this->*Gen1Renderer::_renderers[this->_currentEvent])(target);

	sf::Text text{this->_font};

	text.setCharacterSize(8);
	text.setOutlineThickness(1);
	text.setFillColor(Gen1Renderer::_getDmgColor(3));
	text.setOutlineColor(Gen1Renderer::_getDmgColor(0));
	text.setString("Turn " + std::to_string(this->_currentTurn));
	text.setPosition({160.f - text.getString().getSize() * 8, 136});
	target.draw(text);
}

void Gen1Renderer::consumeEvent(const Event &event)
{
	this->_queue.push_back(event);
}

std::optional<IRenderer::BattleAction> Gen1Renderer::selectAction(bool attackDisabled)
{
	return {};
}

void Gen1Renderer::reset()
{
}

const sf::Texture &Gen1Renderer::getPkmnFace(unsigned int pkmnId)
{
	auto it = this->_data.find(pkmnId);

	if (it == this->_data.end()) {
		this->_missingno.front.palettize({0, 1, 2, 3}, false);
		return this->_missingno.front.texture;
	}
	it->second.front.palettize({0, 1, 2, 3}, false);
	return it->second.front.texture;
}

const sf::SoundBuffer &Gen1Renderer::getPkmnCry(unsigned int pkmnId)
{
	auto it = this->_data.find(pkmnId);

	if (it == this->_data.end())
		return this->_missingno.cry;
	return it->second.cry;
}

sf::Vector2u Gen1Renderer::getSize() const
{
	return { 160, 144 };
}

static std::string splitText(std::string str)
{
	size_t lineSize = 0;
	std::string result;
	std::string token;

	result.reserve(str.size());
	for (size_t pos = str.find(' '); !str.empty(); pos = str.find(' ')) {
		token = str.substr(0, pos);

		if (!result.empty()) {
			if (lineSize + token.size() + 1 > 18) {
				result += "\n";
				lineSize = 0;
			} else {
				result += " ";
				lineSize++;
			}
		}
		result += token;
		lineSize += token.size();
		if (pos == std::string::npos)
			break;
		str.erase(0, pos + 1);
	}
	return result;
}

void Gen1Renderer::_handleEvent(const Event &event)
{
	if (std::get_if<TurnStartEvent>(&event)) {
		this->_currentEvent = EVNTTYPE_START_TURN;
		this->_animCounter = 0;
	} else if (std::get_if<GameStartEvent>(&event)) {
		this->_currentEvent = EVNTTYPE_GAME_START;
		this->_animCounter = 0;
		this->_animMove = 0;
		this->_subCounter = 0;
		this->_currentAnim = 0;
	} else if (auto move = std::get_if<MoveEvent>(&event)) {
		this->_currentEvent = EVNTTYPE_MOVE;
		if (move->moveId > 165)
			throw std::runtime_error("Move not implemented");

		auto &f = move->player ? this->state.p1 : this->state.p2;
		auto it = this->_moveData.find(move->moveId);
		auto it2 = this->_data.find(f.team[f.active].id);
		auto &pkmn = it2 == this->_data.end() ? this->_missingno : it2->second;

		this->_isPlayer = move->player;
		this->_animMove = move->moveId;
		this->_animCounter = 0;
		this->_subCounter = 0;
		this->_subSpawnTimer = 0;
		this->_subSpawnUnspawn = 0;
		this->_waitCounter = 0;
		if (move->moveId == 45)
			this->_moveSound.setBuffer(pkmn.growl);
		else if (move->moveId == 46)
			this->_moveSound.setBuffer(pkmn.roar);
		else
			this->_moveSound.setBuffer(it->second.sound);
	} else if (auto text = std::get_if<TextEvent>(&event)) {
		this->_currentEvent = EVNTTYPE_TEXT;
		this->_currentCharacter = 0;
		this->_textTimer = 0;
		this->_textCounter = 0;
		this->_displayedText.clear();
		this->_queuedText = splitText(text->message);
	} else if (auto withdraw = std::get_if<WithdrawEvent>(&event)) {
		this->_currentEvent = EVNTTYPE_WITHDRAW;
		this->_isPlayer = withdraw->player;
		this->_animCounter = 0;
	} else if (auto death = std::get_if<DeathEvent>(&event)) {
		this->_currentEvent = EVNTTYPE_DEATH;
		this->_isPlayer = death->player;
		this->_animCounter = 0;
	} else if (auto switch_ = std::get_if<SwitchEvent>(&event)) {
		this->_currentEvent = EVNTTYPE_SWITCH;
		this->_isPlayer = switch_->player;
		if (switch_->player) {
			this->state.p1.active = switch_->newPkmnId;
			this->state.p1.spriteId = this->state.p1.team[switch_->newPkmnId].id;
			this->state.p1.hidden = false;
			this->_currentAnim = 0;
			this->_subCounter = 0;
			this->_animCounter = 0;
		} else {
			this->state.p2.active = switch_->newPkmnId;
			this->state.p2.spriteId = this->state.p2.team[switch_->newPkmnId].id;
			this->state.p2.hidden = false;
			this->_currentAnim = this->_ballPopAnim.size();
			this->_animCounter = 40;
		}
	} else if (auto hit = std::get_if<HitEvent>(&event)) {
		this->_currentEvent = EVNTTYPE_HIT;
		this->_isPlayer = hit->player;
		this->_notVeryEffective = hit->notVeryEffective;
		this->_veryEffective = hit->veryEffective;
		this->_animCounter = 0;
	} else if (auto health = std::get_if<HealthModEvent>(&event)) {
		this->_currentEvent = EVNTTYPE_HEALTH_MOD;
		this->_isPlayer = health->player;
		this->_healthTarget = health->newHealth;
	} else if (auto anim = std::get_if<AnimEvent>(&event)) {
		auto &state = anim->player ? this->state.p1 : this->state.p2;
		auto &pkmn = state.team[state.active];

		this->_currentEvent = EVNTTYPE_ANIM;
		this->_animCounter = 0;
		if (anim->animId >= SYSANIM_ATK_DECREASE_BIG && anim->animId <= SYSANIM_EVD_INCREASE_BIG) {
			if (anim->player == anim->turn)
				this->_currentEvent = EVNTTYPE_NONE;
			else if (!anim->isGuaranteed)
				this->_currentAnim = ANIMTYPE_DELAY;
			// In gen1 you can only lower the stats from your opponent as a side effect, so we assume it was lowered
			else if (anim->player)
				this->_currentAnim = ANIMTYPE_STAT_LOWER_PLAYER;
			else if (!anim->player)
				this->_currentAnim = ANIMTYPE_STAT_LOWER_OPPONENT;
		} else if (anim->animId >= SYSANIM_NOW_ASLEEP && anim->animId < SYSANIM_NOW_CONFUSED) {
			if (anim->animId != SYSANIM_NOW_CONFUSED) {
				pkmn.asleep = anim->animId == SYSANIM_NOW_ASLEEP;
				pkmn.frozen = anim->animId == SYSANIM_NOW_FROZEN;
				pkmn.burned = anim->animId == SYSANIM_NOW_BURNED;
				pkmn.poisoned = anim->animId == SYSANIM_NOW_POISONED;
				pkmn.paralyzed = anim->animId == SYSANIM_NOW_PARALYZED;
				pkmn.toxicPoisoned = anim->animId == SYSANIM_NOW_BADLY_POISONED;
			}
			if (anim->isGuaranteed) {
				if (anim->player)
					this->_currentAnim = ANIMTYPE_STAT_LOWER_PLAYER;
				else
					this->_currentAnim = ANIMTYPE_STAT_LOWER_OPPONENT;
			} else if (!anim->player)
				this->_currentAnim = ANIMTYPE_STATUS_SIDE_EFFECT_LOWER_OPPONENT;
			else
				throw std::runtime_error("Status anim not implemented: " + std::to_string(anim->animId) + " not guaranteed");
		} else if (anim->animId >= SYSANIM_ASLEEP && anim->animId <= SYSANIM_LEECHED) {
			this->_currentEvent = EVNTTYPE_MOVE;
			this->_isPlayer = anim->player;
			this->_animCounter = 0;
			this->_subCounter = 0;
			this->_subSpawnTimer = 0;
			this->_subSpawnUnspawn = 0;
			this->_waitCounter = 0;
			if (anim->animId == SYSANIM_CONFUSED) {
				this->_animMove = Amnesia;
				this->_moveSound.setBuffer(this->_moveData[Amnesia].sound);
			} else if (anim->animId == SYSANIM_CONFUSED_HIT) {
				this->_animMove = Pound;
				this->_moveSound.setBuffer(this->_moveData[Pound].sound);
				this->_isPlayer = !anim->player;
			} else if (anim->animId == SYSANIM_ASLEEP) {
				this->_animMove = Rest;
				this->_moveSound.setBuffer(this->_moveData[Rest].sound);
			} else if (anim->animId == SYSANIM_LEECHED) {
				this->_animMove = Absorb;
				this->_moveSound.setBuffer(this->_moveData[Absorb].sound);
			} else if (anim->animId == SYSANIM_POISON || anim->animId == SYSANIM_BURN) {
				this->_animMove = 186;
				this->_moveSound.setBuffer(this->_moveData[186].sound);
			} else {
				this->_currentAnim = ANIMTYPE_DELAY;
				this->_currentEvent = EVNTTYPE_ANIM;
			}
		} else if (anim->animId == SYSANIM_WAKE_UP) {
			this->_currentAnim = ANIMTYPE_DELAY;
			pkmn.asleep = false;
		} else if (anim->animId == SYSANIM_BACK_TO_SENSE) {
			this->_currentAnim = ANIMTYPE_DELAY;
			pkmn.confused = false;
		} else if (anim->animId == SYSANIM_THAWED) {
			this->_currentAnim = ANIMTYPE_DELAY;
			pkmn.frozen = false;
		} else if (anim->animId == SYSANIM_SUB_BREAK) {
			this->_currentAnim = ANIMTYPE_DELAY;
			state.substitute = false;
		} else if (anim->animId == SYSANIM_RECHARGE || anim->animId == SYSANIM_NOW_CONFUSED)
			this->_currentEvent = EVNTTYPE_NONE;
		else
			throw std::runtime_error("Anim not implemented: " + std::to_string(anim->animId));
	} else if (auto extraAnim = std::get_if<ExtraAnimEvent>(&event)) {
		this->_currentEvent = EVNTTYPE_MOVE;
		this->_isPlayer = extraAnim->player;
		this->_animCounter = 0;
		this->_subCounter = 0;
		this->_subSpawnTimer = 0;
		this->_subSpawnUnspawn = 0;
		this->_waitCounter = 0;
		if (
			extraAnim->moveId == Take_Down ||
			extraAnim->moveId == Double_Edge ||
			extraAnim->moveId == Submission ||
			extraAnim->moveId == Struggle ||
			extraAnim->moveId == Hi_Jump_Kick ||
			extraAnim->moveId == Jump_Kick
		) {
			this->_currentEvent = EVNTTYPE_ANIM;
			this->_currentAnim = ANIMTYPE_DELAY;
		} else if (extraAnim->moveId == Skull_Bash || extraAnim->moveId == Solarbeam || extraAnim->moveId == Sky_Attack) {
			this->_animMove = Growth;
			this->_moveSound.setBuffer(this->_noSound);
		} else if (extraAnim->moveId == Fly) {
			this->_animMove = Teleport;
			this->_moveSound.setBuffer(this->_moveData[Teleport].sound);
		} else if (extraAnim->moveId == Dig) {
			this->_animMove = 192;
			this->_moveSound.setBuffer(this->_moveData[192].sound);
		} else
			throw std::runtime_error("Extra anim not implemented: " + std::to_string(extraAnim->moveId));
	} else
		throw std::runtime_error("Not implemented: " + std::to_string(event.index()));
}

void Gen1Renderer::_peakTextEvent()
{
	if (this->_queue.empty())
		return;

	auto text = std::get_if<TextEvent>(&this->_queue.front());

	if (!text)
		return;
	this->_displayedText.clear();
	this->_queuedText = splitText(text->message);
	this->_queue.pop_front();
}

enum IntroStep {
	INTROSTEP_VS_PANEL,
	INTROSTEP_VS_PANEL_BLINK,
	INTROSTEP_VS_PANEL_OUT,
	INTROSTEP_VS_PANEL_OUT_WAIT,
	INTROSTEP_PLAYERS_SLIDE,
	INTROSTEP_PLAYERS_STARE,
	INTROSTEP_PLAYERS_STARE_BALLS,
	INTROSTEP_OPPONENT_SLIDE,
	INTROSTEP_OPPONENT_SLIDE_WAIT,
	INTROSTEP_OPPONENT_MON_TEXT,
	INTROSTEP_OPPONENT_MON_SPAWN,
	INTROSTEP_OPPONENT_MON_SPAWNED_WAIT,
	INTROSTEP_PLAYER_SLIDE,
	INTROSTEP_PLAYER_SLIDE_WAIT,
	INTROSTEP_PLAYER_MON_TEXT,
	INTROSTEP_PLAYER_BALL_ANIM,
	INTROSTEP_PLAYER_MON_SPAWN,
	INTROSTEP_PLAYER_MON_SPAWNED_WAIT,
};

static unsigned introAnimCounters[] = {
	/* INTROSTEP_VS_PANEL                  */ 180,
	/* INTROSTEP_VS_PANEL_BLINK            */ 120,
	/* INTROSTEP_VS_PANEL_OUT              */ 1,
	/* INTROSTEP_VS_PANEL_OUT_WAIT         */ 10,
	/* INTROSTEP_PLAYERS_SLIDE             */ 90,
	/* INTROSTEP_PLAYERS_STARE             */ 60,
	/* INTROSTEP_PLAYERS_STARE_BALLS       */ 90,
	/* INTROSTEP_OPPONENT_SLIDE            */ 15,
	/* INTROSTEP_OPPONENT_SLIDE_WAIT       */ 15,
	/* INTROSTEP_OPPONENT_MON_TEXT         */ 60,
	/* INTROSTEP_OPPONENT_MON_SPAWN        */ 12,
	/* INTROSTEP_OPPONENT_MON_SPAWNED_WAIT */ 78,
	/* INTROSTEP_PLAYER_SLIDE              */ 15,
	/* INTROSTEP_PLAYER_SLIDE_WAIT         */ 15,
	/* INTROSTEP_PLAYER_MON_TEXT           */ 30,
	/* INTROSTEP_PLAYER_BALL_ANIM          */ 40,
	/* INTROSTEP_PLAYER_MON_SPAWN          */ 12,
	/* INTROSTEP_PLAYER_MON_SPAWNED_WAIT   */ 60
};
static std::array<std::vector<sf::Vector2f>, 6> valuesHit{
	std::vector<sf::Vector2f>{ // Not very effective - player
		{8, 0}, {8, 0}, {8, 0}, {8, 0}, {8, 0},
		{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
		{7, 0}, {7, 0}, {7, 0}, {7, 0},
		{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
		{6, 0}, {6, 0}, {6, 0}, {6, 0},
		{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
		{5, 0}, {5, 0}, {5, 0}, {5, 0},
		{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
		{4, 0}, {4, 0}, {4, 0}, {4, 0},
		{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
		{3, 0}, {3, 0}, {3, 0}, {3, 0},
		{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
		{2, 0}, {2, 0}, {2, 0}, {2, 0},
		{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
		{1, 0}, {1, 0}, {1, 0}, {1, 0}
	},
	std::vector<sf::Vector2f>{ // Effective - player
		{0, 8}, {0, 8}, {0, 8}, {0, 8},
		{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
		{0, 7}, {0, 7}, {0, 7},
		{0, 0}, {0, 0}, {0, 0},
		{0, 6}, {0, 6}, {0, 6},
		{0, 0}, {0, 0}, {0, 0},
		{0, 5}, {0, 5}, {0, 5},
		{0, 0}, {0, 0}, {0, 0},
		{0, 4}, {0, 4}, {0, 4},
		{0, 0}, {0, 0}, {0, 0},
		{0, 3}, {0, 3}, {0, 3},
		{0, 0}, {0, 0}, {0, 0},
		{0, 2}, {0, 2}, {0, 2},
		{0, 0}, {0, 0}, {0, 0},
		{0, 1}, {0, 1}, {0, 1}, {0, 1}
	},
	std::vector<sf::Vector2f>{ // Very effective - player
		{0, 8}, {0, 8}, {0, 8}, {0, 8},
		{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
		{0, 7}, {0, 7}, {0, 7},
		{0, 0}, {0, 0}, {0, 0},
		{0, 6}, {0, 6}, {0, 6},
		{0, 0}, {0, 0}, {0, 0},
		{0, 5}, {0, 5}, {0, 5},
		{0, 0}, {0, 0}, {0, 0},
		{0, 4}, {0, 4}, {0, 4},
		{0, 0}, {0, 0}, {0, 0},
		{0, 3}, {0, 3}, {0, 3},
		{0, 0}, {0, 0}, {0, 0},
		{0, 2}, {0, 2}, {0, 2},
		{0, 0}, {0, 0}, {0, 0},
		{0, 1}, {0, 1}, {0, 1}, {0, 1}
	},
	std::vector<sf::Vector2f>{ // Not very effective - opponent
		{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
		{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
		{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
		{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	},
	std::vector<sf::Vector2f>{ // Effective - opponent
		{2, 0}, {2, 0}, {2, 0}, {2, 0}, {2, 0},
		{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
		{1, 0}, {1, 0}, {1, 0}, {1, 0}
	},
	std::vector<sf::Vector2f>{ // Very effective - opponent
		{2, 0}, {2, 0}, {2, 0}, {2, 0}, {2, 0},
		{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
		{1, 0}, {1, 0}, {1, 0}, {1, 0}
	},
};
// Side effect status as player: 2, 2, -2, -2 (total 8 times)

bool Gen1Renderer::_updateNormal()
{
	return false;
}
bool Gen1Renderer::_updateTurnStart()
{
	if (this->_animCounter++ < 60)
		return true;
	this->_currentTurn++;
	return false;
}
bool Gen1Renderer::_updateGameStart()
{
	if (this->_skipping) {
		this->_music.setPlayingOffset(this->_music.getLoopPoints().offset);
		this->_music.play();
		return false;
	}
	if (++this->_animCounter >= introAnimCounters[this->_animMove]) {
		this->_animMove++;
		this->_animCounter = 0;
	}

	switch (this->_animMove) {
	case INTROSTEP_VS_PANEL:
		if (this->_animCounter == 1 && !this->musicDisabled)
			this->_music.play();
		break;
	case INTROSTEP_PLAYERS_STARE:
		if (this->_animCounter == 0 && !this->soundDisabled)
			this->_soundLand.play();
		break;
	case INTROSTEP_PLAYERS_STARE_BALLS:
	case INTROSTEP_OPPONENT_MON_TEXT:
	case INTROSTEP_PLAYER_MON_TEXT:
		if (this->_animCounter == 0)
			this->_peakTextEvent();
		break;
	case INTROSTEP_OPPONENT_MON_SPAWN:
		if (this->_animCounter == 10) {
			auto it = this->_data.find(this->state.p2.spriteId);
			auto &data = it == this->_data.end() ? this->_missingno : it->second;

			this->_crySound.setBuffer(data.cry);
			if (!this->soundDisabled)
				this->_crySound.play();
		}
		break;
	case INTROSTEP_PLAYER_BALL_ANIM:
		if (this->_currentAnim < this->_ballPopAnim.size()) {
			this->_subCounter++;
			if (this->_subCounter >= this->_ballPopAnim[this->_currentAnim].duration) {
				this->_currentAnim++;
				this->_subCounter = 0;
			}
		}
		if (this->_animCounter == 0 && !this->soundDisabled)
			this->_ballPop.play();
		break;
	case INTROSTEP_PLAYER_MON_SPAWN:
		if (this->_animCounter == 10) {
			auto it = this->_data.find(this->state.p1.spriteId);
			auto &data = it == this->_data.end() ? this->_missingno : it->second;

			this->_crySound.setBuffer(data.cry);
			if (!this->soundDisabled)
				this->_crySound.play();
			this->_displayedText = this->_queuedText;
		}
		break;
	}

	return this->_animMove < std::size(introAnimCounters);
}
bool Gen1Renderer::_updateGameEnd()
{
	return false;
}
bool Gen1Renderer::_updateHit()
{
	if (this->_animCounter == 0) {
		if (this->_notVeryEffective)
			this->_hitSound.setBuffer(this->_hitSounds[0]);
		else if (this->_veryEffective)
			this->_hitSound.setBuffer(this->_hitSounds[2]);
		else
			this->_hitSound.setBuffer(this->_hitSounds[1]);
		if (!this->soundDisabled)
			this->_hitSound.play();
	}

	unsigned index = (!this->_isPlayer * 3) + (this->_veryEffective) + (!this->_notVeryEffective);

	if (valuesHit[index].empty())
		throw std::runtime_error("Hit not implemented");
	return this->_animCounter++ < valuesHit[index].size();
}
bool Gen1Renderer::_updateDeath()
{
	if (this->_animCounter == 40) {
		this->_faintSound.setBuffer(this->_faint);
		if (!this->soundDisabled)
			this->_faintSound.play();
	}
	if (this->_animCounter++ < 75)
		return true;
	if (this->_isPlayer)
		this->state.p1.hidden = true;
	else
		this->state.p2.hidden = true;
	return false;
}
bool Gen1Renderer::_updateSwitch()
{
	if (this->_currentAnim < this->_ballPopAnim.size()) {
		this->_subCounter++;
		if (this->_subCounter >= this->_ballPopAnim[this->_currentAnim].duration) {
			this->_currentAnim++;
			this->_subCounter = 0;
		}
	}
	if (this->_animCounter == 0 && !this->soundDisabled)
		this->_ballPop.play();
	if (this->_animCounter == 50) {
		auto it = this->_data.find(this->_isPlayer ? this->state.p1.spriteId : this->state.p2.spriteId);
		auto &data = it == this->_data.end() ? this->_missingno : it->second;

		this->_crySound.setBuffer(data.cry);
		if (!this->soundDisabled)
			this->_crySound.play();
		this->_displayedText = this->_queuedText;
	}
	return this->_animCounter++ < 120;
}
bool Gen1Renderer::_updateWithdraw()
{
	if (this->_animCounter++ < WITHDRAW_ANIM_LENGTH)
		return true;
	if (this->_isPlayer) {
		this->state.p1.hidden = true;
		this->state.p1.substitute = false;
	} else {
		this->state.p2.hidden = true;
		this->state.p2.substitute = false;
	}
	return false;
}
bool Gen1Renderer::_updateHealthMod()
{
	auto &p = (this->_isPlayer ? this->state.p1 : this->state.p2);

	if (p.team[p.active].hp < this->_healthTarget)
		p.team[p.active].hp++;
	else if (p.team[p.active].hp > this->_healthTarget)
		p.team[p.active].hp--;
	return p.team[p.active].hp != this->_healthTarget;
}
bool Gen1Renderer::_updateExtraAnim()
{
	return false;
}
bool Gen1Renderer::_updateAnim()
{
	switch (this->_currentAnim) {
	case ANIMTYPE_DELAY:
		return this->_animCounter++ < TEXT_WAIT_AFTER;
	case ANIMTYPE_STAT_LOWER_PLAYER:
		return this->_animCounter++ < 46;
	case ANIMTYPE_STAT_LOWER_OPPONENT:
		return this->_animCounter++ < 22;
	case ANIMTYPE_STATUS_SIDE_EFFECT_LOWER_OPPONENT:
		return this->_animCounter++ < 32;
	}
	return false;
}
bool Gen1Renderer::_updateMove()
{
	auto it = this->_moveData.find(this->_animMove);
	auto &state = this->_isPlayer ? this->state.p1 : this->state.p2;
	auto &anim = this->_isPlayer ? it->second.animP1 : it->second.animP2;

	if (this->_animCounter == 0) {
		if (this->_waitCounter < MOVE_WAIT_BEFORE) {
			this->_waitCounter++;
			return true;
		}
		if (state.substitute && this->_subSpawnTimer < 16) {
			this->_subSpawnTimer++;
			return true;
		}
	} else if (anim.size() == this->_animCounter) {
		if (state.substitute && this->_subSpawnTimer < 16) {
			this->_subSpawnTimer++;
			return this->_subSpawnTimer < 16;
		}
	}

	if (this->_animCounter == 0 && this->_subCounter == 0 && !this->soundDisabled)
		this->_moveSound.play();
	this->_subCounter++;
	if (this->_subCounter == anim[this->_animCounter].duration) {
		auto &ostate = this->_isPlayer ? this->state.p2 : this->state.p1;

		this->_subCounter = 0;
		this->_animCounter++;
		if (this->_animMove == 164) { // Substitute
			if (this->_animCounter == anim.size() - 2)
				state.substitute = true;
		} else if (this->_animMove == 107) { // Minimize
			if (this->_animCounter == anim.size() - 3)
				state.spriteId = 257;
		} else if (this->_animMove == 144) { // Transform
			if (this->_animCounter == anim.size() - 2)
				state.spriteId = ostate.team[ostate.active].id;
		}
	}
	if (anim.size() == this->_animCounter) {
		if (this->_isPlayer)
			this->state.p1.hidden = !anim.back().p1Off.has_value();
		else
			this->state.p2.hidden = !anim.back().p2Off.has_value();
		if (state.substitute && this->_animMove != 164) {
			this->_subSpawnTimer = 0;
			this->_subSpawnUnspawn = 1;
			return true;
		}
		return false;
	}
	return true;
}
bool Gen1Renderer::_updateText()
{
	if (this->_queuedText.empty())
		return false;
	if (this->_currentCharacter == this->_queuedText.size()) {
		if (!this->_queue.empty() && std::holds_alternative<TextEvent>(this->_queue.front())) {
			this->_textTimer++;
			return this->_textTimer < TEXT_WAIT_AFTER;
		}
		return false;
	}
	//if (this->_textTimer < 4) {
	//	this->_textTimer++;
	//	return true;
	//}
	if (this->_queuedText[this->_currentCharacter] == '\n' && this->_displayedText.find('\n') != std::string::npos) {
		this->_textCounter++;
		if (this->_textCounter == TEXT_LINE_TIME) {
			auto pos = this->_displayedText.find('\n');

			this->_textCounter = 0;
			this->_displayedText.erase(0, pos + 1);
		} else
			return true;
	}
	this->_displayedText += this->_queuedText[this->_currentCharacter];
	this->_currentCharacter++;
	return true;
}

void Gen1Renderer::_displayMyStats(sf::RenderTarget &target, const Pokemon &pkmn, const std::array<unsigned, 4> &palette)
{
	float percent = static_cast<float>(pkmn.hp) / pkmn.maxHp;
	sf::Sprite sprite{this->_boxes[3].texture};
	sf::RectangleShape rect;
	sf::Color healthColor;
	sf::Text text{this->_font};

	this->_boxes[3].palettize(palette, false);
	text.setCharacterSize(8);
	text.setOutlineThickness(0);
	text.setFillColor(Gen1Renderer::_getDmgColor(palette[3]));
	sprite.setPosition({72, 72});
	target.draw(sprite);

	if (!this->_hasColor)
		healthColor = Gen1Renderer::_getDmgColor(palette[2]);
	else if (percent >= 0.5)
		healthColor = sf::Color::Green;
	else if (percent >= 0.1)
		healthColor = sf::Color{255, 128, 0, 255};
	else
		healthColor = sf::Color::Red;
	rect.setFillColor(healthColor);
	rect.setSize({std::round(48 * percent), 4});
	rect.setPosition({95, 73});
	target.draw(rect);

	this->_hpOverlay.palettize(palette, false);
	sprite.setTexture(this->_hpOverlay.texture, true);
	sprite.setPosition({80, 72});
	target.draw(sprite);

	text.setPosition({112, 64});
	if (pkmn.burned) {
		text.setString("BRN");
		target.draw(text);
	} else if (pkmn.poisoned || pkmn.toxicPoisoned) {
		text.setString("PSN");
		target.draw(text);
	} else if (pkmn.frozen) {
		text.setString("FRZ");
		target.draw(text);
	} else if (pkmn.asleep) {
		text.setString("SLP");
		target.draw(text);
	} else if (pkmn.paralyzed) {
		text.setString("PAR");
		target.draw(text);
	} else {
		text.setString(std::to_string(pkmn.level));
		if (pkmn.level < 100) {
			sprite.setPosition({104, 64});
			this->_levelSprite.palettize(palette, false);
			sprite.setTexture(this->_levelSprite.texture, true);
			target.draw(sprite);
			text.setPosition({112, 64});
			target.draw(text);
		} else
			target.draw(text);
	}

	text.setString(pkmn.name);
	text.setPosition({80, 56});
	target.draw(text);

	text.setString(std::to_string(pkmn.hp));
	text.setPosition({112.f - std::to_string(pkmn.hp).size() * 8, 80});
	target.draw(text);
	text.setString("/");
	text.setPosition({112.f, 80});
	target.draw(text);
	text.setString(std::to_string(pkmn.maxHp));
	text.setPosition({144.f - std::to_string(pkmn.maxHp).size() * 8, 80});
	target.draw(text);
}

void Gen1Renderer::_displayOpStats(sf::RenderTarget &target, const Pokemon &pkmn, const std::array<unsigned, 4> &palette)
{
	float percent = static_cast<float>(pkmn.hp) / pkmn.maxHp;
	sf::Sprite sprite{this->_boxes[2].texture};
	sf::RectangleShape rect;
	sf::Color healthColor;
	sf::Text text{this->_font};

	this->_boxes[2].palettize(palette, false);
	text.setCharacterSize(8);
	text.setOutlineThickness(0);
	text.setFillColor(Gen1Renderer::_getDmgColor(palette[3]));
	sprite.setPosition({8, 16});
	target.draw(sprite);

	if (!this->_hasColor)
		healthColor = Gen1Renderer::_getDmgColor(palette[2]);
	else if (percent >= 0.5)
		healthColor = sf::Color::Green;
	else if (percent >= 0.1)
		healthColor = sf::Color{255, 128, 0, 255};
	else
		healthColor = sf::Color::Red;
	rect.setFillColor(healthColor);
	rect.setSize({std::round(48 * percent), 4});
	rect.setPosition({31, 17});
	target.draw(rect);

	this->_hpOverlay.palettize(palette, false);
	sprite.setTexture(this->_hpOverlay.texture, true);
	sprite.setPosition({16, 16});
	target.draw(sprite);

	text.setPosition({32, 8});
	if (pkmn.burned) {
		text.setString("BRN");
		target.draw(text);
	} else if (pkmn.poisoned || pkmn.toxicPoisoned) {
		text.setString("PSN");
		target.draw(text);
	} else if (pkmn.frozen) {
		text.setString("FRZ");
		target.draw(text);
	} else if (pkmn.asleep) {
		text.setString("SLP");
		target.draw(text);
	} else if (pkmn.paralyzed) {
		text.setString("PAR");
		target.draw(text);
	} else {
		text.setString(std::to_string(pkmn.level));
		if (pkmn.level < 100) {
			sprite.setPosition({32, 8});
			this->_levelSprite.palettize(palette, false);
			sprite.setTexture(this->_levelSprite.texture, true);
			target.draw(sprite);
			text.setPosition({40, 8});
			target.draw(text);
		} else
			target.draw(text);
	}

	text.setString(pkmn.name);
	text.setPosition({8, 0});
	target.draw(text);
}

void Gen1Renderer::_renderScene(sf::RenderTarget &target, const std::array<unsigned int, 4> &palette)
{
	sf::Text text{this->_font};
	sf::Sprite sprite{this->_boxes[0].texture};

	target.clear(Gen1Renderer::_getDmgColor(palette[0]));
	this->_displayMyStats(target, this->state.p1.team[this->state.p1.active], palette);
	this->_displayOpStats(target, this->state.p2.team[this->state.p2.active], palette);

	this->_boxes[0].palettize(palette, false);
	sprite.setPosition({0, 96});
	target.draw(sprite);

	text.setCharacterSize(8);
	text.setOutlineThickness(0);
	text.setFillColor(Gen1Renderer::_getDmgColor(palette[3]));
	text.setString(this->_displayedText);
	text.setPosition({8, 112});
	text.setLineSpacing(2);
	target.draw(text);
}

void Gen1Renderer::_displayMyFace(sf::RenderTarget &target, unsigned pkmnId, const std::array<unsigned int, 4> &palette, const sf::Vector2i &offset)
{
	if (this->state.p1.hidden)
		return;

	sf::Vector2f basePos{8 + offset.x * 8.f, 40};
	auto it = this->_data.find(pkmnId);
	auto &data = it == this->_data.end() ? this->_missingno : it->second;
	sf::Sprite sprite{data.back.texture};
	auto size = data.back.texture.getSize();

	if (offset.y > 0) {
		basePos.y += offset.y * 8.f;
		sprite.setTextureRect({{0, 0}, {static_cast<int>(size.x), static_cast<int>(size.y) - offset.y * 4 - 4}});
	} else if (offset.y < 0)
		sprite.setTextureRect({
			{0, static_cast<int>(-offset.y * 4.f)},
			{static_cast<int>(size.x), static_cast<int>(size.y) + offset.y * 4}
		});
	data.back.palettize(palette, true);
	if (pkmnId < 256)
		sprite.setScale({2, 2});
	sprite.setPosition(basePos);
	target.draw(sprite);
}

void Gen1Renderer::_displayOpFace(sf::RenderTarget &target, unsigned pkmnId, const std::array<unsigned int, 4> &palette, const sf::Vector2i &offset)
{
	if (this->state.p2.hidden)
		return;

	sf::Vector2f basePos{96 + offset.x * 8.f, 0};
	auto it = this->_data.find(pkmnId);
	auto &data = it == this->_data.end() ? this->_missingno : it->second;
	sf::Sprite sprite{data.front.texture};
	auto size = data.front.source.getSize();

	if (offset.y > 0) {
		basePos.y += offset.y * 8.f;
		sprite.setTextureRect({{0, 0}, {static_cast<int>(size.x), static_cast<int>(size.y) - offset.y * 8}});
	} else if (offset.y < 0)
		sprite.setTextureRect({
			{0, static_cast<int>(-offset.y * 8.f)},
			{static_cast<int>(size.x), static_cast<int>(size.y) + offset.y * 8}
		});
	basePos.x += static_cast<int>(56.f - size.x) / 16 * 8;
	basePos.y += 56 - size.y;
	data.front.palettize(palette, true);
	sprite.setPosition(basePos);
	target.draw(sprite);
}

void Gen1Renderer::_displayMyShrunkFace(sf::RenderTarget &target, unsigned int pkmnId, const std::array<unsigned int, 4> &palette, unsigned int current, unsigned int max)
{
	if (current >= max)
		return;

	sf::Vector2f basePos{8, 40};
	auto it = this->_data.find(pkmnId);
	auto &data = it == this->_data.end() ? this->_missingno : it->second;
	sf::Sprite sprite{data.back.texture};
	auto size = data.back.texture.getSize();
	auto realSize = size;

	data.back.palettize(palette, true);
	if (pkmnId < 256) {
		sprite.setScale({2, 2});
		realSize.x *= 2;
		realSize.y *= 2;
	}
	if (current < max / 2) {
		sprite.setTextureRect({
			{0, 0},
			{static_cast<int>((size.x / 2) - (size.x / 2) * current / (max / 2)), static_cast<int>(size.y)}
		});
		sprite.setPosition({basePos.x + (realSize.x / 2) * current / (max / 2), basePos.y});
		target.draw(sprite);

		sprite.setTextureRect({
			{static_cast<int>(size.x / 2), 0},
			{static_cast<int>((size.x + 1) / 2), static_cast<int>(size.y)}
		});
		sprite.setPosition({basePos.x + realSize.x / 2, basePos.y});
		target.draw(sprite);
	} else {
		sprite.setTextureRect({
			{static_cast<int>(size.x / 2), 0},
			{static_cast<int>(((size.x + 1) / 2) - ((size.x + 1) / 2) * (current - max / 2) / (max / 2)), static_cast<int>(size.y)}
		});
		sprite.setPosition({basePos.x + realSize.x / 2, basePos.y});
		target.draw(sprite);
	}
}

void Gen1Renderer::_displayOpShrunkFace(sf::RenderTarget &target, unsigned int pkmnId, const std::array<unsigned int, 4> &palette, unsigned int current, unsigned int max)
{
	if (current >= max)
		return;

	sf::Vector2f basePos{96, 0};
	auto it = this->_data.find(pkmnId);
	auto &data = it == this->_data.end() ? this->_missingno : it->second;
	sf::Sprite sprite{data.front.texture};
	auto size = data.front.source.getSize();
	auto realSize = size;

	basePos.x += static_cast<int>(56.f - size.x) / 16 * 8;
	basePos.y += 56 - size.y;
	data.front.palettize(palette, true);
	if (current < max / 2) {
		sprite.setTextureRect({
			{0, 0},
			{static_cast<int>((size.x / 2) - (size.x / 2) * current / (max / 2)), static_cast<int>(size.y)}
		});
		sprite.setPosition({basePos.x + (realSize.x / 2) * current / (max / 2), basePos.y});
		target.draw(sprite);

		sprite.setTextureRect({
			{static_cast<int>(size.x / 2), 0},
			{static_cast<int>((size.x + 1) / 2), static_cast<int>(size.y)}
		});
		sprite.setPosition({basePos.x + realSize.x / 2, basePos.y});
		target.draw(sprite);
	} else {
		sprite.setTextureRect({
			{static_cast<int>(size.x / 2), 0},
			{static_cast<int>(((size.x + 1) / 2) - ((size.x + 1) / 2) * (current - max / 2) / (max / 2)), static_cast<int>(size.y)}
		});
		sprite.setPosition({basePos.x + realSize.x / 2, basePos.y});
		target.draw(sprite);
	}
}

void Gen1Renderer::_renderNormal(sf::RenderTarget &target)
{
	this->_renderScene(target);
	this->_displayMyFace(target, this->state.p1.substitute ? 256 : this->state.p1.spriteId);
	this->_displayOpFace(target, this->state.p2.substitute ? 256 : this->state.p2.spriteId);

	/*if (menu == 0) {
		drawSprite(window, sprite, resources.choicesHUD, 256, 384);
		drawSprite(window, sprite, resources.arrows[1], 288 + 192 * (selectedMenu % 2), 448 + 64 * (selectedMenu / 2));
		if (ai1 && state.me.nextAction == EmptyAction)
			state.me.nextAction = ai1->getNextMove(state, false);
		if (ai2 && state.op.nextAction == EmptyAction)
			state.op.nextAction = ai2->getNextMove(state, true);
	} else if (menu == 1) {
		auto move = state.me.team[state.me.pokemonOnField].getMoveSet()[selectedMenu];

		drawSprite(window, sprite, resources.attackHUD, 0, 256);
		for (int i = 0; i < 4; i++)
			drawText(window, text, strToUpper(state.me.team[state.me.pokemonOnField].getMoveSet()[i].getName()), 192, 416 + 32 * i);
		drawText(window, text, strToUpper(typeToString(move.getType())), 64, 320);
		drawText(window, text, std::to_string(move.getPP()), 160 + (move.getPP() < 10) * 32, 352);
		drawText(window, text, "/" + std::to_string(move.getMaxPP()), 224, 352);
		drawText(window, text, std::to_string(move.getMaxPP()), 256 + (move.getMaxPP() < 10) * 32, 352);
		drawSprite(window, sprite, resources.arrows[1], 160, 416 + 32 * selectedMenu);
	} else if (menu == 2) {
		window.clear({255, 255, 255, 255});
		drawSprite(window, sprite, resources.boxes[0], 0, 384);
		for (size_t i = 0; i < state.me.team.size(); i++) {
			auto &pkmn = state.me.team[i];
			float percent = static_cast<float>(pkmn.getHealth()) / pkmn.getMaxHealth();

			rect.setFillColor(
				percent >= 0.5 ? sf::Color{0, 255, 0, 255} : (
					percent >= 0.1 ? sf::Color{255, 255, 0, 255} : sf::Color{255, 0, 0, 255})
			);
			rect.setSize({192 * percent, 16});
			rect.setPosition({188, i * 64 + 36.f});
			window.draw(rect);
			drawSprite(window, sprite, resources.hpOverlay, 128, 32 + i * 64);
			if (pkmn.getHealth() == 0) {
				drawText(window, text, "FNT", 416, i * 64);
			} else if (pkmn.hasStatus(STATUS_BURNED)) {
				drawText(window, text, "BRN", 416, i * 64);
			} else if (pkmn.hasStatus(STATUS_POISONED) || pkmn.hasStatus(STATUS_BADLY_POISONED)) {
				drawText(window, text, "PSN", 416, i * 64);
			} else if (pkmn.hasStatus(STATUS_FROZEN)) {
				drawText(window, text, "FRZ", 416, i * 64);
			} else if (pkmn.hasStatus(STATUS_ASLEEP)) {
				drawText(window, text, "SLP", 416, i * 64);
			} else if (pkmn.hasStatus(STATUS_PARALYZED)) {
				drawText(window, text, "PAR", 416, i * 64);
			} else {
				drawSprite(window, sprite, resources.levelSprite, 416, i * 64);
				drawText(window, text, std::to_string(pkmn.getLevel()), 448, i * 64);
			}
			drawText(window, text, pkmn.getName(), 96, i * 64);

			drawText(window, text, std::to_string(pkmn.getHealth()), 512 - std::to_string(pkmn.getHealth()).size() * 32, i * 64 + 32);
			drawText(window, text, "/", 512, 64 * 2 + 32);
			drawText(window, text, std::to_string(pkmn.getMaxHealth()), 640 - std::to_string(pkmn.getMaxHealth()).size() * 32, i * 64 + 32);
		}
		drawSprite(window, sprite, resources.arrows[1], 0, 64 * selectedMenu + 32);
	} else if (menu == 3) {
		if (log.empty()) {
			drawSprite(window, sprite, resources.waitingHUD, 96, 320);
			drawText(window, text, "Waiting...", 128, 352);
			if (updateManually && (state.op.nextAction || game.playingReplay()))
				state.onTurnStart();
		} else {
			clock.restart();
			menu = 4 + (state.me.team[state.me.pokemonOnField].getHealth() == 0) * 2;
		}
	} else if (menu >= 4) {
		drawText(window, text, log[0].substr(0, clock.getElapsedTime().asSeconds() * 50), 32, 440);
		if (clock.getElapsedTime().asSeconds() > log[0].size() / 50.f + 1) {
			log.erase(log.begin());
			if (log.empty())
				menu -= 4;
			else
				clock.restart();
		}
	}*/
}
void Gen1Renderer::_renderTurnStart(sf::RenderTarget &target)
{
	this->_renderNormal(target);
	if (this->_animCounter < 20 || this->_animCounter >= 50)
		return;
	this->_waitingHUD.palettize({0, 1, 2, 3}, false);

	sf::Sprite sprite{this->_waitingHUD.texture};
	sf::Text text{this->_font};

	sprite.setPosition({24, 80});
	target.draw(sprite);
	text.setString("Waiting...");
	text.setPosition({32, 88});
	text.setCharacterSize(8);
	text.setFillColor(Gen1Renderer::_getDmgColor(3));
	text.setLineSpacing(2);
	target.draw(text);
}
void Gen1Renderer::_renderGameStart(sf::RenderTarget &target)
{
	sf::Text text{this->_font};
	sf::Sprite sprite{this->_boxes[0].texture};

	text.setCharacterSize(8);
	text.setFillColor(Gen1Renderer::_getDmgColor(3));
	text.setLineSpacing(2);

	target.clear(Gen1Renderer::_getDmgColor(0));
	sprite.setPosition({0, 96});
	target.draw(sprite);
	switch (this->_animMove){
	case INTROSTEP_VS_PANEL:
		target.clear(Gen1Renderer::_getDmgColor(0));

		sprite.setTexture(this->_boxes[1].texture, true);
		sprite.setPosition({24, 36});
		target.draw(sprite);

		text.setString(this->state.p1.name);
		text.setPosition({36, 48});
		target.draw(text);
		for (unsigned i = 0; i < this->state.p1.team.size(); i++) {
			auto &pkmn = this->state.p1.team[i];

			if (pkmn.id == 0)
				sprite.setTexture(this->_balls[1], true);
			else if (pkmn.ko)
				sprite.setTexture(this->_balls[2], true);
			else if (pkmn.asleep || pkmn.frozen || pkmn.burned || pkmn.poisoned || pkmn.toxicPoisoned || pkmn.paralyzed)
				sprite.setTexture(this->_balls[3], true);
			else
				sprite.setTexture(this->_balls[0], true);
			sprite.setPosition({72 + i * 8.f, 58});
			target.draw(sprite);
		}

		text.setString(this->state.p2.name);
		text.setPosition({36, 80});
		target.draw(text);
		for (unsigned i = 0; i < state.p2.team.size(); i++) {
			auto &pkmn = this->state.p2.team[i];

			if (pkmn.id == 0)
				sprite.setTexture(this->_balls[1], true);
			else if (pkmn.ko)
				sprite.setTexture(this->_balls[2], true);
			else if (pkmn.asleep || pkmn.frozen || pkmn.burned || pkmn.poisoned || pkmn.toxicPoisoned || pkmn.paralyzed)
				sprite.setTexture(this->_balls[3], true);
			else
				sprite.setTexture(this->_balls[0], true);
			sprite.setPosition({72 + i * 8.f, 88});
			target.draw(sprite);
		}
		break;

	case INTROSTEP_VS_PANEL_BLINK:
	#define BLINK_STEP 5
		if (this->_animCounter % (BLINK_STEP * 6) < BLINK_STEP)
			target.clear(Gen1Renderer::_getDmgColor(0));
		else if (this->_animCounter % (BLINK_STEP * 6) < BLINK_STEP * 2)
			target.clear(Gen1Renderer::_getDmgColor(1));
		else if (this->_animCounter % (BLINK_STEP * 6) < BLINK_STEP * 3)
			target.clear(Gen1Renderer::_getDmgColor(2));
		else if (this->_animCounter % (BLINK_STEP * 6) < BLINK_STEP * 4)
			target.clear(Gen1Renderer::_getDmgColor(3));
		else if (this->_animCounter % (BLINK_STEP * 6) < BLINK_STEP * 5)
			target.clear(Gen1Renderer::_getDmgColor(2));
		else
			target.clear(Gen1Renderer::_getDmgColor(1));
		break;

	case INTROSTEP_VS_PANEL_OUT:
	case INTROSTEP_VS_PANEL_OUT_WAIT:
		target.clear(Gen1Renderer::_getDmgColor(3));
		break;

	case INTROSTEP_PLAYERS_SLIDE:
		this->_trainer[1].palettize({3, 3, 3, 3}, true);
		this->_trainer[0].palettize({3, 3, 3, 3}, true);

		sprite.setTexture(this->_trainer[1].texture, true);
		sprite.setPosition({158.f * this->_animCounter / introAnimCounters[INTROSTEP_PLAYERS_SLIDE] - 56, 0});
		target.draw(sprite);

		sprite.setTexture(this->_trainer[0].texture, true);
		sprite.setScale({2, 2});
		sprite.setPosition({160.f * (introAnimCounters[INTROSTEP_PLAYERS_SLIDE] - this->_animCounter) / introAnimCounters[INTROSTEP_PLAYERS_SLIDE] + 8, 40});
		target.draw(sprite);
		break;

	case INTROSTEP_PLAYERS_STARE:
		this->_trainer[1].palettize({0, 1, 2, 3}, true);
		this->_trainer[0].palettize({0, 1, 2, 3}, true);

		sprite.setTexture(this->_trainer[1].texture, true);
		sprite.setPosition({102, 0});
		target.draw(sprite);

		sprite.setScale({2, 2});
		sprite.setTexture(this->_trainer[0].texture, true);
		sprite.setPosition({8, 40});
		target.draw(sprite);
		break;

	case INTROSTEP_PLAYERS_STARE_BALLS:
		this->_trainer[1].palettize({0, 1, 2, 3}, true);
		this->_trainer[0].palettize({0, 1, 2, 3}, true);
		this->_boxes[2].palettize({0, 1, 2, 3}, true);

		sprite.setTexture(this->_trainer[1].texture, true);
		sprite.setPosition({102, 0});
		target.draw(sprite);

		sprite.setScale({2, 2});
		sprite.setTexture(this->_trainer[0].texture, true);
		sprite.setPosition({8, 40});
		target.draw(sprite);


		sprite.setScale({1, 1});
		sprite.setTexture(this->_boxes[2].texture, true);
		sprite.setPosition({8, 16});
		target.draw(sprite);
		for (unsigned i = 0; i < this->state.p2.team.size(); i++) {
			auto &pkmn = this->state.p2.team[i];

			if (pkmn.id == 0)
				sprite.setTexture(this->_balls[1], true);
			else if (pkmn.ko)
				sprite.setTexture(this->_balls[2], true);
			else if (pkmn.asleep || pkmn.frozen || pkmn.burned || pkmn.poisoned || pkmn.toxicPoisoned || pkmn.paralyzed)
				sprite.setTexture(this->_balls[3], true);
			else
				sprite.setTexture(this->_balls[0], true);
			sprite.setPosition({16 + 8.f * (6.f - i), 16});
			target.draw(sprite);
		}

		sprite.setTexture(this->_boxes[2].texture, true);
		sprite.setPosition({152, 80});
		sprite.setScale({-1, 1});
		target.draw(sprite);
		sprite.setScale({1, 1});
		for (unsigned i = 0; i < this->state.p1.team.size(); i++) {
			auto &pkmn = this->state.p1.team[i];

			if (pkmn.id == 0)
				sprite.setTexture(this->_balls[1], true);
			else if (pkmn.ko)
				sprite.setTexture(this->_balls[2], true);
			else if (pkmn.asleep || pkmn.frozen || pkmn.burned || pkmn.poisoned || pkmn.toxicPoisoned || pkmn.paralyzed)
				sprite.setTexture(this->_balls[3], true);
			else
				sprite.setTexture(this->_balls[0], true);
			sprite.setPosition({88 + 8.f * i, 80});
			target.draw(sprite);
		}

		text.setString(this->_queuedText.substr(0, this->_animCounter));
		text.setPosition({8, 112});
		target.draw(text);
		break;

	case INTROSTEP_OPPONENT_SLIDE:
		this->_trainer[1].palettize({0, 1, 2, 3}, true);
		this->_trainer[0].palettize({0, 1, 2, 3}, true);

		sprite.setTexture(this->_trainer[1].texture, true);
		sprite.setPosition({102 + 224 * this->_animCounter / 60.f, 0});
		target.draw(sprite);

		sprite.setScale({2, 2});
		sprite.setTexture(this->_trainer[0].texture, true);
		sprite.setPosition({8, 40});
		target.draw(sprite);
		break;

	case INTROSTEP_OPPONENT_SLIDE_WAIT:
		this->_trainer[0].palettize({0, 1, 2, 3}, true);

		sprite.setScale({2, 2});
		sprite.setTexture(this->_trainer[0].texture, true);
		sprite.setPosition({8, 40});
		target.draw(sprite);
		break;

	case INTROSTEP_OPPONENT_MON_TEXT:
		this->_trainer[0].palettize({0, 1, 2, 3}, true);

		sprite.setScale({2, 2});
		sprite.setTexture(this->_trainer[0].texture, true);
		sprite.setPosition({8, 40});
		target.draw(sprite);

		text.setString(this->_queuedText.substr(0, this->_animCounter));
		text.setPosition({8, 112});
		target.draw(text);
		break;

	case INTROSTEP_OPPONENT_MON_SPAWN: {
		auto it = this->_data.find(this->state.p2.spriteId);
		auto &data = it == this->_data.end() ? this->_missingno : it->second;

		sf::Vector2f basePos{96, 0};
		float mul = 5.f * this->_animCounter / 60.f;
		auto size = data.front.texture.getSize();

		basePos.x += static_cast<int>(56.f - size.x) / 16 * 8;
		basePos.y += 56 - size.y;

		data.front.palettize({0, 1, 2, 3}, true);
		sprite.setTexture(data.front.texture, true);
		sprite.setScale({mul, mul});
		sprite.setPosition({
			basePos.x + (size.x - size.x * mul) / 2,
			basePos.y + size.y - size.y * mul
		});
		target.draw(sprite);

		this->_trainer[0].palettize({0, 1, 2, 3}, true);

		sprite.setScale({2, 2});
		sprite.setTexture(this->_trainer[0].texture, true);
		sprite.setPosition({8, 40});
		target.draw(sprite);

		text.setString(this->_queuedText);
		text.setPosition({8, 112});
		target.draw(text);
		break;
	}

	case INTROSTEP_OPPONENT_MON_SPAWNED_WAIT:
		this->_displayOpFace(target, this->state.p2.spriteId);

		this->_trainer[0].palettize({0, 1, 2, 3}, true);

		sprite.setScale({2, 2});
		sprite.setTexture(this->_trainer[0].texture, true);
		sprite.setPosition({8, 40});
		target.draw(sprite);

		text.setString(this->_queuedText);
		text.setPosition({8, 112});
		target.draw(text);
		break;

	case INTROSTEP_PLAYER_MON_TEXT:
		text.setString(this->_queuedText.substr(0, this->_animCounter));
		text.setPosition({8, 112});
		target.draw(text);

		this->_displayOpFace(target, this->state.p2.spriteId);
		this->_displayOpStats(target, this->state.p2.team[this->state.p2.active]);
		break;

	case INTROSTEP_PLAYER_SLIDE:
		this->_trainer[0].palettize({0, 1, 2, 3}, true);

		sprite.setScale({2, 2});
		sprite.setTexture(this->_trainer[0].texture, true);
		sprite.setPosition({2 - 64 * this->_animCounter * 5 / 60.f, 40});
		target.draw(sprite);

		this->_displayOpFace(target, this->state.p2.spriteId);
		this->_displayOpStats(target, this->state.p2.team[this->state.p2.active]);

		text.setString(this->_queuedText);
		text.setPosition({8, 112});
		target.draw(text);
		break;

	case INTROSTEP_PLAYER_SLIDE_WAIT:
		text.setString(this->_queuedText);
		text.setPosition({8, 112});
		target.draw(text);

		this->_displayOpFace(target, this->state.p2.spriteId);
		this->_displayOpStats(target, this->state.p2.team[this->state.p2.active]);
		break;

	case INTROSTEP_PLAYER_BALL_ANIM: {
		text.setString(this->_queuedText);
		text.setPosition({8, 112});
		target.draw(text);

		this->_displayOpFace(target, this->state.p2.spriteId);
		this->_displayMyStats(target, this->state.p1.team[this->state.p1.active]);
		this->_displayOpStats(target, this->state.p2.team[this->state.p2.active]);

		if (this->_currentAnim >= this->_ballPopAnim.size())
			break;

		auto &frame = this->_ballPopAnim[this->_currentAnim];
		auto &tileset = frame.tileset == 1 ? this->_moveTextures[0] : this->_moveTextures[1];

		sprite.setTexture(tileset.texture);
		sprite.setOrigin({4, 4});
		tileset.palettize({0, 1, 2, 3}, true);
		for (auto &s : frame.sprites) {
			sprite.setTextureRect({
				{static_cast<int>(s.id % 16) * 8, static_cast<int>(s.id / 16) * 8},
				{8, 8}
			});
			sprite.setPosition({s.x + 4.f, s.y + 4.f});
			sprite.setScale({s.flip.first ? -1.f : 1.f, s.flip.second ? -1.f : 1.f});
			target.draw(sprite);
		}
		break;
	}

	case INTROSTEP_PLAYER_MON_SPAWN: {
		auto it = this->_data.find(this->state.p1.spriteId);
		auto &data = it == this->_data.end() ? this->_missingno : it->second;
		float mul = 5.f * this->_animCounter / 60.f;
		auto size = data.back.texture.getSize();

		size.y -= 4;
		size.x *= 2;
		size.y *= 2;
		data.back.palettize({0, 1, 2, 3}, true);
		sprite.setTexture(data.back.texture, true);
		sprite.setScale({mul * 2, mul * 2});
		sprite.setPosition({
			8 + (size.x - size.x * mul) / 2,
			40 + size.y - size.y * mul
		});
		target.draw(sprite);

		text.setString(this->_queuedText);
		text.setPosition({8, 112});
		target.draw(text);

		this->_displayOpFace(target, this->state.p2.spriteId);
		this->_displayOpStats(target, this->state.p2.team[this->state.p2.active]);
		this->_displayMyStats(target, this->state.p1.team[this->state.p1.active]);
		break;
	}
	case INTROSTEP_PLAYER_MON_SPAWNED_WAIT:
		this->_renderNormal(target);
	}
}
void Gen1Renderer::_renderGameEnd(sf::RenderTarget &target)
{
}
void Gen1Renderer::_renderHit(sf::RenderTarget &target)
{
	auto size = this->getSize();
	sf::RenderTexture rtexture{size};
	sf::Sprite sprite{rtexture.getTexture()};
	unsigned index = (!this->_isPlayer * 3) + (this->_veryEffective) + (!this->_notVeryEffective);
	sf::Vector2f pos = valuesHit[index][this->_animCounter - 1];

	pos.y += size.y;
	this->_renderScene(rtexture);
	this->_displayMyFace(rtexture, this->state.p1.substitute ? 256 : this->state.p1.spriteId);
	if (!this->_isPlayer && this->_notVeryEffective) {
		if (this->_animCounter % 8 < 4)
			this->_displayOpFace(rtexture, this->state.p2.substitute ? 256 : this->state.p2.spriteId);
	} else
		this->_displayOpFace(rtexture, this->state.p2.substitute ? 256 : this->state.p2.spriteId);
	sprite.setScale({1, -1});
	sprite.setPosition(pos);
	target.clear(Gen1Renderer::_getDmgColor(0));
	target.draw(sprite);
}
void Gen1Renderer::_renderDeath(sf::RenderTarget &target)
{
	this->_renderScene(target);
	if (this->_animCounter > 40) {
		if (this->_isPlayer)
			this->_displayOpFace(target, this->state.p2.spriteId);
		else
			this->_displayMyFace(target, this->state.p1.spriteId);

		int index = (this->_animCounter - 40) / 2;

		if (index < 7) {
			if (this->_isPlayer)
				this->_displayMyFace(target, this->state.p1.spriteId, {0, 1, 2, 3}, {0, index});
			else
				this->_displayOpFace(target, this->state.p2.spriteId, {0, 1, 2, 3}, {0, index});
		}
	} else {
		this->_displayMyFace(target, this->state.p1.spriteId);
		this->_displayOpFace(target, this->state.p2.spriteId);
	}
}
void Gen1Renderer::_renderSwitch(sf::RenderTarget &target)
{
	this->_renderScene(target);

	if (this->_currentAnim < this->_ballPopAnim.size()) {
		auto &frame = this->_ballPopAnim[this->_currentAnim];
		auto &tileset = frame.tileset == 1 ? this->_moveTextures[0] : this->_moveTextures[1];
		sf::Sprite sprite{tileset.texture};

		sprite.setOrigin({4, 4});
		tileset.palettize({0, 1, 2, 3}, true);
		for (auto &s : frame.sprites) {
			sprite.setTextureRect({
				{static_cast<int>(s.id % 16) * 8, static_cast<int>(s.id / 16) * 8},
				{8, 8}
			});
			sprite.setPosition({s.x + 4.f, s.y + 4.f});
			sprite.setScale({s.flip.first ? -1.f : 1.f, s.flip.second ? -1.f : 1.f});
			target.draw(sprite);
		}

		this->_displayOpFace(target, this->state.p2.spriteId);
		return;
	}

	float mul = 5.f * (this->_animCounter - 40.f) / 60.f;

	if (mul < 0) {
		if (this->_isPlayer)
			this->_displayOpFace(target, this->state.p2.spriteId);
		else
			this->_displayMyFace(target, this->state.p1.spriteId);
	} else if (mul > 1) {
		this->_displayOpFace(target, this->state.p2.spriteId);
		this->_displayMyFace(target, this->state.p1.spriteId);
	} else if (this->_isPlayer) {
		auto it = this->_data.find(this->state.p1.spriteId);
		auto &data = it == this->_data.end() ? this->_missingno : it->second;
		auto size = data.back.texture.getSize();
		sf::Sprite sprite{data.back.texture};

		size.y -= 4;
		size.x *= 2;
		size.y *= 2;
		data.back.palettize({0, 1, 2, 3}, true);
		sprite.setScale({mul * 2, mul * 2});
		sprite.setPosition({
			8 + (size.x - size.x * mul) / 2,
			40 + size.y - size.y * mul
		});
		target.draw(sprite);
		this->_displayOpFace(target, this->state.p2.spriteId);
	} else {
		auto it = this->_data.find(this->state.p2.spriteId);
		auto &data = it == this->_data.end() ? this->_missingno : it->second;
		sf::Vector2f basePos{96, 0};
		auto size = data.front.texture.getSize();
		sf::Sprite sprite{data.front.texture};

		basePos.x += static_cast<int>(56.f - size.x) / 16 * 8;
		basePos.y += 56 - size.y;

		data.front.palettize({0, 1, 2, 3}, true);
		sprite.setScale({mul, mul});
		sprite.setPosition({
			basePos.x + (size.x - size.x * mul) / 2,
			basePos.y + size.y - size.y * mul
		});
		target.draw(sprite);
		this->_displayMyFace(target, this->state.p1.spriteId);
	}
}
void Gen1Renderer::_renderWithdraw(sf::RenderTarget &target)
{
	sf::Sprite sprite{this->_boxes[0].texture};
	float mul = 1 - (5.f * this->_animCounter / 60.f);

	this->_renderScene(target);
	if (this->_isPlayer) {
		if (mul > 0) {
			auto it = this->_data.find(this->state.p1.spriteId);
			auto &data = it == this->_data.end() ? this->_missingno : it->second;
			auto size = data.back.texture.getSize();

			size.y -= 4;
			size.x *= 2;
			size.y *= 2;
			data.back.palettize({0, 1, 2, 3}, true);
			sprite.setTexture(data.back.texture, true);
			sprite.setScale({mul * 2, mul * 2});
			sprite.setPosition({
				8 + (size.x - size.x * mul) / 2,
				40 + size.y - size.y * mul
			});
			target.draw(sprite);
		}

		this->_displayOpFace(target, this->state.p2.spriteId);
	} else {
		if (mul > 0) {
			auto it = this->_data.find(this->state.p2.spriteId);
			auto &data = it == this->_data.end() ? this->_missingno : it->second;
			sf::Vector2f basePos{96, 0};
			auto size = data.front.texture.getSize();

			basePos.x += static_cast<int>(56.f - size.x) / 16 * 8;
			basePos.y += 56 - size.y;

			data.front.palettize({0, 1, 2, 3}, true);
			sprite.setTexture(data.front.texture, true);
			sprite.setScale({mul, mul});
			sprite.setPosition({
				basePos.x + (size.x - size.x * mul) / 2,
				basePos.y + size.y - size.y * mul
			});
			target.draw(sprite);
		}

		this->_displayMyFace(target, this->state.p1.spriteId);
	}
}
void Gen1Renderer::_renderHealthMod(sf::RenderTarget &target)
{
	this->_renderNormal(target);
}
void Gen1Renderer::_renderExtraAnim(sf::RenderTarget &target)
{
}
void Gen1Renderer::_renderAnim(sf::RenderTarget &target)
{
	switch (this->_currentAnim) {
	case ANIMTYPE_DELAY:
		return this->_renderNormal(target);
	case ANIMTYPE_STAT_LOWER_PLAYER: {
		auto size = this->getSize();
		sf::RenderTexture rtexture{size};
		sf::Sprite sprite{rtexture.getTexture()};
		std::array<float, 23> values{1, 2, 3, 4, 5, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 5, 4, 3, 2, 1};

		this->_renderScene(rtexture);
		this->_displayMyFace(rtexture, this->state.p1.substitute ? 256 : this->state.p1.spriteId);
		this->_displayOpFace(rtexture, this->state.p2.substitute ? 256 : this->state.p2.spriteId);
		sprite.setScale({1, -1});
		sprite.setPosition({values[(this->_animCounter - 1) / 2], static_cast<float>(size.y)});
		target.clear(Gen1Renderer::_getDmgColor(0));
		target.draw(sprite);
		return;
	}
	case ANIMTYPE_STAT_LOWER_OPPONENT: {
		auto size = this->getSize();
		sf::RenderTexture rtexture{size};
		sf::Sprite sprite{rtexture.getTexture()};
		std::array<float, 11> values{1, 2, 3, 2, 1, 0, 1, 2, 3, 2, 1};

		this->_renderScene(rtexture);
		this->_displayMyFace(rtexture, this->state.p1.substitute ? 256 : this->state.p1.spriteId);
		this->_displayOpFace(rtexture, this->state.p2.substitute ? 256 : this->state.p2.spriteId);
		sprite.setScale({1, -1});
		sprite.setPosition({values[(this->_animCounter - 1) / 2], static_cast<float>(size.y)});
		target.clear(Gen1Renderer::_getDmgColor(0));
		target.draw(sprite);
		return;
	}
	case ANIMTYPE_STATUS_SIDE_EFFECT_LOWER_OPPONENT: {
		auto size = this->getSize();
		sf::RenderTexture rtexture{size};
		sf::Sprite sprite2{rtexture.getTexture()};
		sf::Text text{this->_font};
		sf::Sprite sprite{this->_boxes[0].texture};

		rtexture.clear(Gen1Renderer::_getDmgColor(0));
		this->_displayOpStats(rtexture, this->state.p2.team[this->state.p2.active]);
		this->_displayOpFace(rtexture, this->state.p2.substitute ? 256 : this->state.p2.spriteId);


		sprite2.setScale({1, -1});
		sprite2.setPosition({(this->_animCounter / 2) % 2 == 0 ? 2.f : -2.f, static_cast<float>(size.y)});
		target.clear(Gen1Renderer::_getDmgColor(0));
		target.draw(sprite2);

		this->_boxes[0].palettize({0, 1, 2, 3}, false);
		sprite.setPosition({0, 96});
		target.draw(sprite);

		this->_displayMyStats(target, this->state.p1.team[this->state.p1.active]);
		this->_displayMyFace(target, this->state.p1.substitute ? 256 : this->state.p1.spriteId);

		text.setCharacterSize(8);
		text.setOutlineThickness(0);
		text.setFillColor(Gen1Renderer::_getDmgColor(3));
		text.setString(this->_displayedText);
		text.setPosition({8, 112});
		text.setLineSpacing(2);
		target.draw(text);
		return;
	}}
}
void Gen1Renderer::_renderMove(sf::RenderTarget &target)
{
	if (this->_animCounter == 0 && this->_waitCounter < MOVE_WAIT_BEFORE)
		return this->_renderNormal(target);

	auto size = this->getSize();
	auto it = this->_moveData.find(this->_animMove);

	target.clear(Gen1Renderer::_getDmgColor(0));

	auto &anim = this->_isPlayer ? it->second.animP1 : it->second.animP2;
	auto &state = this->_isPlayer ? this->state.p1 : this->state.p2;
	auto p1s = this->state.p1.substitute && (this->_animMove == 164 || !this->_isPlayer) ? 256 : this->state.p1.spriteId;
	auto p2s = this->state.p2.substitute && (this->_animMove == 164 || this->_isPlayer)  ? 256 : this->state.p2.spriteId;

	if (this->_animCounter == 0) {
		if (state.substitute && this->_subSpawnTimer < 16) {
			this->_renderScene(target, {0, 1, 2, 3});
			this->_displayMyFace(target, this->_isPlayer  ? 256 : p1s, {0, 1, 2, 3}, this->_isPlayer ? sf::Vector2i{-static_cast<int>(this->_subSpawnTimer / 2), 0} : sf::Vector2i{0, 0});
			this->_displayOpFace(target, !this->_isPlayer ? 256 : p2s, {0, 1, 2, 3}, !this->_isPlayer ? sf::Vector2i{static_cast<int>(this->_subSpawnTimer / 2), 0} : sf::Vector2i{0, 0});
			return;
		}
	} else if (anim.size() == this->_animCounter) {
		if (state.substitute && this->_subSpawnTimer < 16) {
			this->_renderScene(target, {0, 1, 2, 3});
			this->_displayMyFace(target, this->_isPlayer  ? this->state.p1.spriteId : p1s, {0, 1, 2, 3}, this->_isPlayer ? sf::Vector2i{-static_cast<int>(this->_subSpawnTimer / 2), 0} : sf::Vector2i{0, 0});
			this->_displayOpFace(target, !this->_isPlayer ? this->state.p2.spriteId : p2s, {0, 1, 2, 3}, !this->_isPlayer ? sf::Vector2i{static_cast<int>(this->_subSpawnTimer / 2), 0} : sf::Vector2i{0, 0});
			return;
		}
	}

	auto &frame = anim[this->_animCounter];

	if (this->_animMove == 100 || this->_animMove == 143) { // Teleport, Sky attack
		if (this->_animCounter == 1) {
			this->_renderScene(target, {0, 1, 2, 3});
			if (frame.p1Off)
				this->_displayMyFace(target, p1s, frame.palB, {frame.p1Off->first, frame.p1Off->second});
			else
				this->_displayMyShrunkFace(target, p1s, frame.palB, this->_subCounter + 1, frame.duration);
			if (frame.p2Off)
				this->_displayOpFace(target, p2s, frame.palB, {frame.p2Off->first, frame.p2Off->second});
			else
				this->_displayOpShrunkFace(target, p2s, frame.palB, this->_subCounter + 1, frame.duration);
			return;
		}
	}

	sf::RenderTexture rtexture{size};
	sf::Sprite sprite{rtexture.getTexture()};
	sf::RenderTexture rtexture2{size};
	sf::Sprite sprite2{rtexture2.getTexture()};

	this->_renderScene(rtexture, frame.palB);
	if (frame.wy >= (int)size.y) {
		float x = frame.scx;
		bool side = frame.wx;

		this->_displayMyFace(rtexture, p1s, frame.palB);
		this->_displayOpFace(rtexture, p2s, frame.palB);
		for (size_t i = 0; i < size.y; i++) {
			sprite.setPosition({x, static_cast<float>(size.y - i - 1)});
			sprite.setTextureRect({
				{ 0, static_cast<int>(i) },
				{ static_cast<int>(size.x), 1 }
			});
			target.draw(sprite);
			if (i % 2 == 0) {
				if (x == -2)
					side = true;
				if (x == 2)
					side = false;
				if (side)
					x++;
				else
					x--;
			}
		}
		return;
	}

	sprite.setPosition({static_cast<float>(frame.wx), static_cast<float>(frame.wy) + this->getSize().y});
	sprite.setScale({1, -1});
	target.draw(sprite);

	auto &tileset = frame.tileset == 1 ? this->_moveTextures[0] : this->_moveTextures[1];

	sprite.setTexture(tileset.texture);
	sprite.setOrigin({4, 4});
	for (auto &s : frame.sprites) {
		if (!s.prio)
			continue;
		tileset.palettize(s.palNum == 0 ? frame.pal0 : frame.pal1, true);
		sprite.setTextureRect({
			{static_cast<int>(s.id % 16) * 8, static_cast<int>(s.id / 16) * 8},
			{8, 8}
		});
		sprite.setPosition({s.x + 4.f, s.y + 4.f});
		sprite.setScale({s.flip.first ? -1.f : 1.f, s.flip.second ? -1.f : 1.f});
		target.draw(sprite);
	}

	rtexture2.clear(sf::Color::Transparent);
	if (frame.p1Off)
		this->_displayMyFace(rtexture2, p1s, frame.palB, {frame.p1Off->first, frame.p1Off->second});
	if (frame.p2Off)
		this->_displayOpFace(rtexture2, p2s, frame.palB, {frame.p2Off->first, frame.p2Off->second});
	sprite2.setPosition({static_cast<float>(frame.wx), static_cast<float>(frame.wy) + this->getSize().y});
	sprite2.setScale({1, -1});
	target.draw(sprite2);

	for (auto &s : frame.sprites) {
		if (s.prio)
			continue;
		tileset.palettize(s.palNum == 0 ? frame.pal0 : frame.pal1, true);
		sprite.setTextureRect({
			{static_cast<int>(s.id % 16) * 8, static_cast<int>(s.id / 16) * 8},
			{8, 8}
		});
		sprite.setPosition({s.x + 4.f, s.y + 4.f});
		sprite.setScale({s.flip.first ? -1.f : 1.f, s.flip.second ? -1.f : 1.f});
		target.draw(sprite);
	}
}
void Gen1Renderer::_renderText(sf::RenderTarget &target)
{
	sf::Text text{this->_font};
	sf::Sprite sprite{this->_boxes[0].texture};

	target.clear(Gen1Renderer::_getDmgColor(0));
	this->_displayMyStats(target, this->state.p1.team[this->state.p1.active]);
	this->_displayOpStats(target, this->state.p2.team[this->state.p2.active]);

	this->_boxes[0].palettize({0, 1, 2, 3}, false);
	sprite.setPosition({0, 96});
	target.draw(sprite);

	text.setCharacterSize(8);
	text.setOutlineThickness(0);
	text.setFillColor(Gen1Renderer::_getDmgColor(3));
	text.setString(this->_displayedText);
	if (this->_textCounter > TEXT_LINE_TIME - TEXT_LINE_SCROLL)
		text.setPosition({8, 104});
	else
		text.setPosition({8, 112});
	text.setLineSpacing(2);
	target.draw(text);
	this->_displayMyFace(target, this->state.p1.substitute ? 256 : this->state.p1.spriteId);
	this->_displayOpFace(target, this->state.p2.substitute ? 256 : this->state.p2.spriteId);
}

void Gen1Renderer::consumeEvent(const sf::Event &event)
{
}

sf::Color Gen1Renderer::_getDmgColor(unsigned int color)
{
	switch (color) {
	case 0:
		return sf::Color{0xFF, 0xFF, 0xFF, 255};
	case 1:
		return sf::Color{0xAA, 0xAA, 0xAA, 255};
	case 2:
		return sf::Color{0x55, 0x55, 0x55, 255};
	default:
		return sf::Color{0x00, 0x00, 0x00, 255};
	}
}

void Gen1Renderer::previousTurn()
{
	throw std::runtime_error("not implemented");
}

void Gen1Renderer::nextTurn()
{
	auto old = this->soundDisabled;
	auto oldT = this->_currentTurn;

	this->soundDisabled = true;
	this->_skipping = true;
	while (oldT == this->_currentTurn)
		this->update();
	this->_skipping = false;
	this->soundDisabled = old;
}

void Gen1Renderer::PalettedSprite::palettize(const std::array<unsigned int, 4> &palette, bool transparent, bool force)
{
	unsigned newPalette = (palette[0] << 0) | (palette[1] << 2) | (palette[2] << 4) | (palette[3] << 6);

	if (newPalette == this->palette && transparent == this->transparent && !force)
		return;
	this->transparent = transparent;
	this->palette = newPalette;

	sf::Image img{this->source.getSize()};
	std::array<sf::Color, 4> pal;

	pal[0] = sf::Color::Transparent;
	for (size_t i = transparent; i < 4; i++)
		pal[i] = Gen1Renderer::_getDmgColor(palette[i]);
	for (unsigned x = 0; x < img.getSize().x; x++)
		for (unsigned y = 0; y < img.getSize().y; y++) {
			auto c = this->source.getPixel({x, y});

			if (c.a == 0) img.setPixel({x, y}, sf::Color::Transparent);
			else if (c.r < 0x40) img.setPixel({x, y}, pal[3]);
			else if (c.r < 0xA0) img.setPixel({x, y}, pal[2]);
			else if (c.r < 0xF0) img.setPixel({x, y}, pal[1]);
			else img.setPixel({x, y}, pal[0]);
		}
	(void)this->texture.loadFromImage(img);
}

void Gen1Renderer::PalettedSprite::init(const std::filesystem::path &path)
{
	(void)this->source.loadFromFile(path);
	(void)this->texture.loadFromImage(this->source);
	this->palettize({0, 1, 2, 3}, false, true);
}
