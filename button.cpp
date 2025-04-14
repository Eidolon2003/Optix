#include "button.h"

bool Button::Update(jaw::AppInterface* app) {
	return false;
}

void Button::Draw(jaw::AppInterface* app) {
	auto pos = jaw::Point((int16_t)x, (int16_t)y);

	uint32_t stroke = (src.br.y - src.tl.y) / 10;

	app->graphics->DrawLine(
		src.tl + pos,
		jaw::Point(src.br.x + pos.x, src.tl.y + pos.y),
		stroke,
		0xFFFFFF,
		layer
	);
	app->graphics->DrawLine(
		src.br + pos,
		jaw::Point(src.br.x + pos.x, src.tl.y + pos.y),
		stroke,
		0xFFFFFF,
		layer
	);
	app->graphics->DrawLine(
		src.tl + pos,
		jaw::Point(src.tl.x + pos.x, src.br.y + pos.y),
		stroke,
		0xFFFFFF,
		layer
	);
	app->graphics->DrawLine(
		src.br + pos,
		jaw::Point(src.tl.x + pos.x, src.br.y + pos.y),
		stroke,
		0xFFFFFF,
		layer
	);
	
	auto font = jaw::Font();
	font.name = L"Consolas";
	font.size = (src.br.y - src.tl.y) / 1.5f;
	font.bold = true;
	font.align = font.CENTER;
	app->graphics->DrawString(
		text,
		jaw::Rect(src.tl + pos, src.br + pos),
		layer,
		font,
		0xFFFFFF
	);
}