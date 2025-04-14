#pragma once
#include <JawEngine.h>
#include "button.h"

class Game final : public jaw::AppInterface {
public:
#pragma pack(push, 1)
#pragma warning(push)
#pragma warning(disable: 26495)
	struct Tile {
		uint16_t id, x, y;
	};

	struct Level {
		struct Header {
			uint32_t magic;
			uint16_t sequence_number;
			uint16_t x, y;
		};
		Header head;
		std::wstring name;
		std::vector<Tile> tiles;
	};
#pragma warning(pop)
#pragma pack(pop)

	Level level;

	//Button handling
	std::vector<std::weak_ptr<Button>> buttons;
	void mouse_handler(jaw::InputInterface::Mouse&);

	//State handling
	typedef enum { NONE, TITLE, LEVEL_SELECT, GAME } State;
	State state;
	void goto_state(State);

	//Title Screen
	void init_title();
	void deinit_title();
	void loop_title();

	//Level Selection Screen
	void init_select();
	void deinit_select();
	void loop_select();

	//In-Game
	void init_game();
	void deinit_game();
	void loop_game();
	void mouse_handler_game(jaw::InputInterface::Mouse&);

	Game();
	void Init() override;
	void Loop() override;
};