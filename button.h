#pragma once
#include <JawEngine.h>

class Button : public jaw::Sprite {
	//The button's size and position are defined by its
	//x, y position and its src rectangle
public:
	std::wstring text = L"";
	virtual bool Update(jaw::AppInterface*) override;
	virtual void Draw(jaw::AppInterface*) override;
	virtual void click(jaw::AppInterface*) = 0;
};