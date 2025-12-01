//
// Created by PinkySmile on 19/11/2025.
//

#ifndef POKEAI_GEN1RENDERER_HPP
#define POKEAI_GEN1RENDERER_HPP


#include <deque>
#include <unordered_map>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include "IRenderer.hpp"

class Gen1Renderer : public IRenderer {
public:
	Gen1Renderer(const std::string &variant);
	~Gen1Renderer() override = default;
	void update() override;
	void render(sf::RenderTarget &) override;
	sf::Vector2u getSize() const override;
	void reset() override;
	void consumeEvent(const PkmnCommon::Event &event) override;
	void consumeEvent(const sf::Event &event) override;
	std::optional<BattleAction> selectAction(bool attackDisabled) override;
	const sf::Texture &getPkmnFace(unsigned int pkmnId) override;
	const sf::SoundBuffer &getPkmnCry(unsigned int pkmnId) override;

private:
	struct PalettedSprite {
		sf::Texture texture;
		sf::Image source;
		unsigned char palette = (0 << 0) | (1 << 2) | (2 << 4) | (3 << 6);
		bool transparent = false;

		void palettize(const std::array<unsigned, 4> &palette, bool transparent, bool force=false);
		void init(const std::filesystem::path &path);
	};

	struct PokemonData {
		sf::SoundBuffer growl;
		sf::SoundBuffer roar;
		sf::SoundBuffer cry;
		PalettedSprite front;
		PalettedSprite back;
		sf::Texture icon;
	};

	struct MoveSprite {
		int x;
		int y;
		unsigned id;
		unsigned palNum;
		std::pair<bool, bool> flip;
		bool prio;
	};

	struct MoveAnim {
		std::array<unsigned, 4> palB;
		std::array<unsigned, 4> pal0;
		std::array<unsigned, 4> pal1;
		unsigned duration;
		int scx;
		int scy;
		int wx;
		int wy;
		std::optional<std::pair<int, int>> p1Off;
		std::optional<std::pair<int, int>> p2Off;
		std::vector<MoveSprite> sprites;
		unsigned tileset;
	};

	struct MoveData {
		std::vector<MoveAnim> animP1;
		std::vector<MoveAnim> animP2;
		sf::SoundBuffer sound;
	};

	enum EventType {
		EVNTTYPE_NONE,
		EVNTTYPE_GAME_START,
		EVNTTYPE_GAME_END,
		EVNTTYPE_HIT,
		EVNTTYPE_DEATH,
		EVNTTYPE_SWITCH,
		EVNTTYPE_WITHDRAW,
		EVNTTYPE_HEALTH_MOD,
		EVNTTYPE_EXTRA_ANIM,
		EVNTTYPE_ANIM,
		EVNTTYPE_MOVE,
		EVNTTYPE_START_TURN,
		EVNTTYPE_TEXT,
		EVNTTYPE_COUNT
	};

	bool _updateNormal();
	bool _updateGameStart();
	bool _updateGameEnd();
	bool _updateHit();
	bool _updateDeath();
	bool _updateSwitch();
	bool _updateWithdraw();
	bool _updateHealthMod();
	bool _updateExtraAnim();
	bool _updateAnim();
	bool _updateMove();
	bool _updateTurnStart();
	bool _updateText();

	void _displayMyStats(sf::RenderTarget &target, const Pokemon &pkmn, const std::array<unsigned, 4> &palette = {0, 1, 2, 3});
	void _displayOpStats(sf::RenderTarget &target, const Pokemon &pkmn, const std::array<unsigned, 4> &palette = {0, 1, 2, 3});
	void _displayMyFace(sf::RenderTarget &target, unsigned pkmnId, const std::array<unsigned, 4> &palette = {0, 1, 2, 3}, const sf::Vector2i &offset = {0, 0});
	void _displayOpFace(sf::RenderTarget &target, unsigned pkmnId, const std::array<unsigned, 4> &palette = {0, 1, 2, 3}, const sf::Vector2i &offset = {0, 0});
	void _displayMyShrunkFace(sf::RenderTarget &target, unsigned pkmnId, const std::array<unsigned, 4> &palette, unsigned current, unsigned max);
	void _displayOpShrunkFace(sf::RenderTarget &target, unsigned pkmnId, const std::array<unsigned, 4> &palette, unsigned current, unsigned max);

	void _renderScene(sf::RenderTarget &, const std::array<unsigned, 4> &palette = {0, 1, 2, 3});
	void _renderNormal(sf::RenderTarget &);
	void _renderGameStart(sf::RenderTarget &);
	void _renderGameEnd(sf::RenderTarget &);
	void _renderHit(sf::RenderTarget &);
	void _renderDeath(sf::RenderTarget &);
	void _renderSwitch(sf::RenderTarget &);
	void _renderWithdraw(sf::RenderTarget &);
	void _renderHealthMod(sf::RenderTarget &);
	void _renderExtraAnim(sf::RenderTarget &);
	void _renderAnim(sf::RenderTarget &);
	void _renderMove(sf::RenderTarget &);
	void _renderTurnStart(sf::RenderTarget &);
	void _renderText(sf::RenderTarget &);

	void _handleEvent(const PkmnCommon::Event &event);
	void _peakTextEvent();

	static sf::Color _getDmgColor(unsigned color);

	static bool (Gen1Renderer::*_updates[EVNTTYPE_COUNT])();
	static void (Gen1Renderer::*_renderers[EVNTTYPE_COUNT])(sf::RenderTarget &);

	static void _loadMoveFrames(std::vector<MoveAnim> &m, const nlohmann::json &j);
	static void _loadMoveData(MoveData &data, const std::string &id);
	static void _loadPokemonData(PokemonData &data, const std::string &folder, const std::string &variant);

	EventType _currentEvent = EVNTTYPE_NONE;

	bool _hasColor = false;
	unsigned _gpCounter[7];
	std::string _queuedText;
	std::string _displayedText;
	sf::Texture _balls[4];
	sf::Texture _arrows[2];
	sf::Texture _choicesHUD;
	sf::Texture _attackHUD;
	PalettedSprite _waitingHUD;
	PalettedSprite _trainer[2];
	PalettedSprite _boxes[4];
	PalettedSprite _hpOverlay;
	PalettedSprite _levelSprite;
	sf::SoundBuffer _hitSounds[3];
	sf::SoundBuffer _trainerLand;
	sf::SoundBuffer _ballPopSound;
	sf::SoundBuffer _menuSelect;
	sf::SoundBuffer _noSound;
	sf::SoundBuffer _faint;
	std::vector<MoveAnim> _ballPopAnim;
	sf::Sound _gpSound{this->_trainerLand};
	sf::Sound _soundLand{this->_trainerLand};
	sf::Sound _ballPop{this->_ballPopSound};
	std::pair<sf::Texture, sf::Texture> _activeMons;
	std::deque<PkmnCommon::Event> _queue;
	std::deque<sf::Event> _sfmlQueue;
	sf::Font _font;
	PokemonData _missingno;
	std::unordered_map<unsigned, PokemonData> _data;
	std::unordered_map<unsigned, MoveData> _moveData;
	sf::Music _music;
	std::array<PalettedSprite, 2> _moveTextures;
};


#endif //POKEAI_GEN1RENDERER_HPP
