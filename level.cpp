#include "game.h"
#include <iostream>
#include <map>

enum { 
	EMPTY,
	WALL,
	GUN,
	TARGET,
	MIRROR_UP, MIRROR_DOWN,
	SPLITTER_LEFT, SPLITTER_UP, SPLITTER_RIGHT, SPLITTER_DOWN
};

jaw::Point grid_offset = { 0,0 };
int tile_scale = 0;
bool show_grid = false;
bool show_ids = false;

union TileStats {
	uint16_t id;
	//Order in HxD: type-color movable-changeable
	struct {
		uint16_t color : 4;
		uint16_t type : 4;
		uint16_t changeable : 4;
		uint16_t movable : 4;
	};
};

std::vector<std::vector<TileStats>> board;
TileStats held_tile{};
bool board_changed = true;
bool win = false;
bool light_oob = false;

//Store light path as a pair of points (src and dest) to a color
std::map<std::pair<jaw::Point, jaw::Point>, uint32_t> light_path;

uint32_t color_eval(uint8_t c) {
	switch (c) {
	case 0:
		return 0xFFFFFF;
	case 1:
		return 0xFF0000;
	case 2:
		return 0x00FF00;
	case 3:
		return 0x0000FF;
	default:
		return 0;
	}
}

void Game::init_game() {
	//Compute appropriate grid positioning and size
	jaw::Point win_size = window->getProperties().size;

	if (level.head.x * 16 > win_size.x * 0.95
		|| level.head.y * 16 > win_size.y * 0.95) 
	{
		engine->ShowCMD(true);
		std::cout << "Error: Window resolution too small for a level of this size";
		engine->CloseWindow(this);
	}

	tile_scale = (int)std::min(
		win_size.x * 0.95 / level.head.x / 16,
		win_size.y * 0.95 / level.head.y / 16
	);

	jaw::Point grid_size(
		level.head.x * tile_scale * 16,
		level.head.y * tile_scale * 16
	);

	grid_offset = (win_size - grid_size) / 2;

	//Start by holding an empty, mutable tile
	held_tile.id = 0;
	held_tile.movable = 1;

	//Fill empty, mutable board
	board.resize(level.head.x);
	for (int i = 0; i < level.head.x; i++) {
		board[i].resize(level.head.y);
	}

	for (int i = 0; i < level.head.x; i++) {
		for (int j = 0; j < level.head.y; j++) {
			board[i][j].id = 0;
			board[i][j].movable = 1;
		}
	}

	bool gun = false;
	bool target = false;

	//Fill board from level data
	for (auto& t : level.tiles) {
		if (t.x >= board.size()
			|| t.y >= board[0].size())
		{
			continue;
		}

		board[t.x][t.y].id = t.id;

		//Make note of where the gun and target are when we see them
		if (board[t.x][t.y].type == GUN) {
			gun = true;
		}
		else if (board[t.x][t.y].type == TARGET) {
			target = true;
		}
	}

	if (!gun || !target)
	{
		engine->ShowCMD(true);
		std::cout << "Error: Level must contain at least a gun and a target";
		engine->CloseWindow(this);
	}

	//Keybinds
	input->BindKeyDown(jaw::G, []() { show_grid = !show_grid; });
	input->BindKeyDown(jaw::I, []() { show_ids = !show_ids; });
	input->BindKeyDown(jaw::M, [this]() { goto_state(LEVEL_SELECT); });

	input->BindKeyDown(jaw::C, []() {
		if (!held_tile.changeable) return;

		if (held_tile.type == GUN || held_tile.type == TARGET || held_tile.type >= SPLITTER_LEFT) {
			held_tile.color++;
			if (held_tile.color > 3) held_tile.color = 0;
		}
	});

	input->BindKeyDown(jaw::R, []() {
		if (held_tile.type == MIRROR_UP) {
			held_tile.type = MIRROR_DOWN;
		}
		else if (held_tile.type == MIRROR_DOWN) {
			held_tile.type = MIRROR_UP;
		}
		else if (held_tile.type >= SPLITTER_LEFT) {
			held_tile.type++;
			if (held_tile.type > SPLITTER_DOWN) {
				held_tile.type = SPLITTER_LEFT;
			}
		}
	});

}

void Game::deinit_game() {
	//Undo keybinds
	input->BindKeyDown(jaw::G, [](){});
	input->BindKeyDown(jaw::I, [](){});
	input->BindKeyDown(jaw::M, [](){});
	input->BindKeyDown(jaw::C, [](){});
	input->BindKeyDown(jaw::R, [](){});

	light_path.clear();
	board_changed = true;
}

void Game::loop_game() {
	auto draw_tile = [this](jaw::Point screen_coord, TileStats t) {
		jaw::Point tile_size = jaw::Point(0, 0) + tile_scale * 16;
		jaw::Point offset(t.color * 16, t.type * 16);

		graphics->DrawPartialBmp(
			graphics->LoadBmp("assets.png"),
			jaw::Rect(screen_coord, screen_coord + tile_size),
			jaw::Rect(offset, offset + 16),
			2
		);
	};

	//Draw Tile BMPs
	for (int i = 0; i < level.head.x; i++) {
		for (int j = 0; j < level.head.y; j++) {
			jaw::Point screen_coord = grid_offset + jaw::Point(i, j) * (tile_scale * 16);
			draw_tile(screen_coord, board[i][j]);
		}
	}

	//Draw held tile
	auto m = input->getMouse();
	draw_tile(m.pos - (tile_scale * 8), held_tile);

	//Draw grid (if applicable)
	if (show_grid) {
		for (int i = 0; i < level.head.x; i++) {
			for (int j = 0; j < level.head.y; j++) {
				jaw::Point tile_coord = grid_offset + (jaw::Point(i, j) * tile_scale * 16);

				graphics->DrawLine(
					tile_coord,
					tile_coord + jaw::Point(tile_scale * 16, 0),
					1,
					0xFFFFFF,
					2
				);
				graphics->DrawLine(
					tile_coord,
					tile_coord + jaw::Point(0, tile_scale * 16),
					1,
					0xFFFFFF,
					2
				);
				graphics->DrawLine(
					tile_coord + jaw::Point(tile_scale * 16, tile_scale * 16),
					tile_coord + jaw::Point(tile_scale * 16, 0),
					1,
					0xFFFFFF,
					2
				);
				graphics->DrawLine(
					tile_coord + jaw::Point(tile_scale * 16, tile_scale * 16),
					tile_coord + jaw::Point(0, tile_scale * 16),
					1,
					0xFFFFFF,
					2
				);
			}
		}
	}

	//Draw Tile IDs (if applicable)
	if (show_ids) {
		for (int i = 0; i < level.head.x; i++) {
			for (int j = 0; j < level.head.y; j++) {
				jaw::Point tile_coord = grid_offset + (jaw::Point(i, j) * tile_scale * 16);

				graphics->DrawString(
					std::to_wstring(board[i][j].id),
					jaw::Rect(tile_coord + 1, tile_coord + tile_scale * 16),
					2
				);
			}
		}
	}

	//Compute a new light path if the board has changed
	std::function<void(jaw::Point, jaw::Point, uint32_t)>
	process_light = [&](jaw::Point s, jaw::Point d, uint32_t c) {
		if (d.x >= board.size() ||
			d.y >= board[0].size())
		{
			light_oob = true;
			return;
		}

		//Light gets blocked when running head-on into another beam
		if (light_path.find(std::make_pair(d, s)) != light_path.end()) {
			return;
		}

		jaw::Point v = d - s;
		TileStats t = board[d.x][d.y];

		auto splitter_handler = [&](jaw::Point front) {
			uint32_t splitter_color = color_eval(t.color);

			//There's definitely a more elegant way to do this
			jaw::Point dv;
			if (front == jaw::Point(1, 0)) {
				dv = jaw::Point(0, -1);
			}
			else if (front == jaw::Point(0, 1)) {
				dv = jaw::Point(1, 0);
			}
			else if (front == jaw::Point(-1, 0)) {
				dv = jaw::Point(0, 1);
			}
			else {
				dv = jaw::Point(-1, 0);
			}

			if (v == front) {
				//Front split
				s = d;

				jaw::Point dp = s + dv;
				jaw::Point dn = s - dv;

				uint32_t cp = c;
				uint32_t cn = c;

				if (splitter_color != 0xFFFFFF) {
					cp &= splitter_color;
					cn &= ~splitter_color;
				}

				light_path.insert_or_assign(
					std::make_pair(s, dp),
					cp
				);
				light_path.insert_or_assign(
					std::make_pair(s, dn),
					cn
				);

				process_light(s, dp, cp);
				process_light(s, dn, cn);
			}
			else if (v == front * -1) {
				//Back wall
				light_oob = true;
			}
			else {
				//Combine sides
				uint32_t c1 = 0;
				uint32_t c2 = 0;

				//Find ray from color side
				auto it = light_path.find(std::make_pair(d + dv, d));
				if (it != light_path.end()) {
					c1 = it->second & splitter_color;
				}

				//Find ray from colorless side
				it = light_path.find(std::make_pair(d - dv, d));
				if (it != light_path.end()) {
					c2 = it->second;
				}

				c = c1 | c2;
				s = d;
				d = s + front * -1;

				if (c) {
					light_path.insert_or_assign(
						std::make_pair(s, d),
						c
					);

					process_light(s, d, c);
				}
			}
		};

		switch (t.type) {
		case WALL:
		case GUN:
			light_oob = true;
		case TARGET:
			return;

		case MIRROR_UP:
			v = v * -1;
		case MIRROR_DOWN:
			std::swap(v.x, v.y);
		case EMPTY:
			s = d;
			d = d + v;
			light_path.insert_or_assign(
				std::make_pair(s, d),
				c
			);
			process_light(s, d, c);
			return;

		case SPLITTER_LEFT:
			splitter_handler(jaw::Point(1, 0));
			return;
		case SPLITTER_UP:
			splitter_handler(jaw::Point(0, 1));
			return;
		case SPLITTER_RIGHT:
			splitter_handler(jaw::Point(-1, 0));
			return;
		case SPLITTER_DOWN:
			splitter_handler(jaw::Point(0, -1));
			return;
		}
	};

	if (board_changed) {
		board_changed = false;
		light_oob = false;
		light_path.clear();

		for (int i = 0; i < level.head.x; i++) {
			for (int j = 0; j < level.head.y; j++) {
				if (board[i][j].type == GUN) {
					jaw::Point gun_pos = jaw::Point(i, j);
					uint32_t gun_color = color_eval(board[i][j].color);

					jaw::Point d = gun_pos + jaw::Point(1, 0);
					light_path.insert_or_assign(
						std::make_pair(gun_pos, d),
						gun_color
					);
					process_light(gun_pos, d, gun_color);
				}
			}
		}

		//Check win condition
		win = false;
		jaw::Point target_pos(-1, -1);
		TileStats target{};
		for (int i = 0; i < level.head.x; i++) {
			for (int j = 0; j < level.head.y; j++) {
				if (board[i][j].type == TARGET) {
					target_pos = jaw::Point(i, j);
					target = board[i][j];
				}
			}
		}
		if (target_pos != jaw::Point(-1, -1)) {
			for (auto& l : light_path) {
				if ((l.first.second == target_pos) &&
					l.second == color_eval(target.color))
				{
					win = true;
				}
			}
		}
	}

	//Draw light path
	for (auto& x : light_path) {
		jaw::Point src = grid_offset + (x.first.first * (tile_scale) * 16) + tile_scale * 8;
		jaw::Point dest = grid_offset + (x.first.second * (tile_scale) * 16) + tile_scale * 8;

		graphics->DrawLine(
			src,
			dest,
			std::max(tile_scale - 1, 1) * 2,
			x.second,
			1
		);
	}

	jaw::Point win_size = window->getProperties().size;
	jaw::Font font = jaw::Font();
	font.align = font.CENTER;
	font.size = win_size.y / 32.f;

	//If won, draw text
	if (win && !light_oob) {
		graphics->DrawString(
			L"Level Complete!",
			jaw::Rect(0, (int)(grid_offset.y * 0.20), win_size.x, win_size.y + (grid_offset.y)),
			2,
			font,
			0xFFFFFF
		);
		graphics->DrawString(
			L"Press M to return to menu.",
			jaw::Rect(0, win_size.y - (int)(grid_offset.y * 0.8), win_size.x, win_size.y),
			2,
			font,
			0xFFFFFF
		);
	}
	//If won, but there isn't only one beam
	else if (win && light_oob) {
		graphics->DrawString(
			L"There can be only one.",
			jaw::Rect(0, (int)(grid_offset.y * 0.20), win_size.x, win_size.y + (grid_offset.y)),
			2,
			font,
			0xFFFFFF
		);
	}
}

void Game::mouse_handler_game(jaw::InputInterface::Mouse& m) {
	if (!m.lmb) return;

	jaw::Point grid_coord = (m.pos - grid_offset) / (tile_scale * 16);

	if (grid_coord.x >= board.size() ||
		grid_coord.y >= board[0].size())
	{
		return;
	}

	TileStats& t = board[grid_coord.x][grid_coord.y];

	if (t.movable) {
		std::swap(held_tile, t);
		board_changed = true;
	}
}