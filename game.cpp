#include "game.h"

Game::Game() {
	state = NONE;
}

void Game::Init() {
	input->BindClickDown([this](auto m) { 
		mouse_handler(m); 
	});

	input->BindKeyDown(jaw::ESC, [this]() {
		engine->CloseWindow(this); 
	});

	goto_state(TITLE);
}

void Game::Loop() {
	switch (state) {
	case NONE:
		break;
	case TITLE:
		loop_title();
		break;
	case LEVEL_SELECT:
		loop_select();
		break;
	case GAME:
		loop_game();
		break;
	}
}

void Game::mouse_handler(jaw::InputInterface::Mouse& m) {
	if (state == GAME) mouse_handler_game(m);

	if (m.lmb) {
		for (auto& x : buttons) {
			Button* p = x.lock().get();
			if (!p) continue;

			if (m.pos.x > (p->x) &&
				m.pos.y > (p->y) &&
				m.pos.x < (p->x + p->src.br.x) &&
				m.pos.y < (p->y + p->src.br.y)
				) 
			{
				p->click(this);
			}
		}
	}
}

void Game::goto_state(State s) {
	switch (state) {
	case NONE:
		break;
	case TITLE:
		deinit_title();
		break;
	case LEVEL_SELECT:
		deinit_select();
		break;
	case GAME:
		deinit_game();
		break;
	}

	state = s;

	switch (state) {
	case NONE:
		break;
	case TITLE:
		init_title();
		break;
	case LEVEL_SELECT:
		init_select();
		break;
	case GAME:
		init_game();
		break;
	}
}