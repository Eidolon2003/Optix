#include "game.h"

class PlayButton : public Button {
public:
	PlayButton(jaw::Point window_size) {
		src = jaw::Rect(0, 0, window_size.x / 3, window_size.y / 10);
		x = (window_size.x - src.br.x) / 2.f;
		y = (window_size.y - src.br.y) / 2.f;
		layer = 1;
		text = L"PLAY";
	}

	void click(jaw::AppInterface* app) override {
		static_cast<Game*>(app)->goto_state(Game::State::LEVEL_SELECT);
	}
};

void Game::init_title() {
	jaw::Point window_size = window->getProperties().size;

	//Draw title text
	auto font = jaw::Font();
	font.bold = true;
	font.align = jaw::Font::CENTER;
	font.name = L"Consolas";
	font.size = window_size.y / 12.f;

	graphics->DrawString(
		L"Optix",
		jaw::Rect(0, window_size.y / 8, window_size.x, window_size.y),
		0,
		font
	);

	font.size = window_size.y / 24.f;
	graphics->DrawString(
		L"Created by Julian Williams",
		jaw::Rect(0, window_size.y * 7 / 8, window_size.x, window_size.y),
		0,
		font
	);

	auto pb = new PlayButton(window_size);
	buttons.push_back(window->RegisterSprite(pb));
}

void Game::deinit_title() {
	for (auto& x : buttons) {
		window->DeregisterSprite(x);
	}
	buttons.clear();

	graphics->ClearLayer(0, 0);
}

void Game::loop_title() {
	return;
}