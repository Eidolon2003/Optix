#include "game.h"
#include <filesystem>
#include <fstream>
#include <vector>

std::vector<Game::Level> levels;

class LevelButton : public Button {
public:
	uint16_t level_index;
	float sx, sy;

	LevelButton(std::wstring title, uint16_t index, jaw::Point win_size) {
		text = title;
		level_index = index;
		x = win_size.x / 4.f;
		sy = (index + 1) * (win_size.y / 5.f);
		y = sy;
		src = jaw::Rect(0, 0, win_size.x / 2, win_size.y / 8);
	}

	virtual void click(jaw::AppInterface* app) override {
		auto m = app->input->getMouse();
		auto h = app->window->getProperties().size.y;
		if (m.pos.y < h / 6)
			return;
		static_cast<Game*>(app)->level = levels[level_index];
		static_cast<Game*>(app)->goto_state(Game::State::GAME);
	}

	virtual bool Update(jaw::AppInterface* app) override {
		auto m = app->input->getMouse();
		auto h = app->window->getProperties().size.y;
		y = sy + m.wheel * h / 16;
		return false;
	}
};

void read_levels(const std::vector<std::filesystem::path>& paths) {
	for (auto& p : paths) {
		if (!p.has_extension() || (p.extension() != ".dat")) continue;

		std::ifstream file(p, std::ios::binary);
		Game::Level l;
		file.read((char*)&l, sizeof(Game::Level::Header));

		if (file.eof() || l.head.magic != 14042025) continue;

		l.tiles = std::vector<Game::Tile>();
		Game::Tile t;
		while (file.read((char*)&t, sizeof(Game::Tile)) && !file.eof()) {
			l.tiles.push_back(t);
		}

		l.name = p.stem().wstring();

		levels.push_back(l);
	}
}

void Game::init_select() {
	//Something going wrong here in release mode
	//input->ResetWheel();

	//Read level data
	//This line will create the directory if it doesn't already exist
	//If it does already exist, the files it contains will remain
	{
		using namespace std::filesystem;

		create_directory("./levels");

		std::vector<path> paths;
		for (auto& x : directory_iterator("./levels")) {
			path path = x.path();
			while (is_symlink(path)) {
				path = read_symlink(path);
			}
			if (is_regular_file(path)) {
				paths.push_back(path);
			}
		}

		read_levels(paths);
	}
	
	//Sort the levels by their sequence numbers
	std::sort(levels.begin(), levels.end(),
		[](Game::Level l, Game::Level r) -> bool {
			return l.head.sequence_number < r.head.sequence_number;
		}
	);
	
	//Create Level Buttons
	jaw::Point win_size = window->getProperties().size;
	for (int i = 0; i < levels.size(); i++) {
		std::wstring title;
		if (levels[i].head.sequence_number > 0) {
			title += std::to_wstring(levels[i].head.sequence_number);
			title += L". ";
		}
		title += levels[i].name;

		buttons.push_back(
			window->RegisterSprite(
				new LevelButton(
					title,
					i,
					win_size
				)
			)
		);
	}
}

void Game::deinit_select() {
	for (auto& x : buttons) {
		window->DeregisterSprite(x);
	}
	buttons.clear();

	levels.clear();
}

void Game::loop_select() {
	jaw::Point win_size = window->getProperties().size;
	graphics->FillRect(
		jaw::Rect(0, 0, win_size.x, win_size.y / 6),
		0x000000,
		2
	);
	
	auto font = jaw::Font();
	font.bold = true;
	font.align = jaw::Font::CENTER;
	font.name = L"Consolas";
	font.size = win_size.y / 12.f;

	graphics->DrawString(
		L"Select a Level",
		jaw::Rect(0, win_size.y / 16, win_size.x, win_size.y),
		2,
		font
	);
}