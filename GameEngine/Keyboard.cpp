#include "Keyboard.h"

Keyboard::Keyboard()
{
}

Keyboard::~Keyboard()
{
}

bool Keyboard::KeyDown(SDL_Event _event, SDL_Keycode _key)
{
	if (_event.type != SDL_KEYDOWN) return false;

	return _key == _event.key.keysym.sym;
}

bool Keyboard::KeyUp(SDL_Event _event, SDL_Keycode _key)
{
	if (_event.type != SDL_KEYUP) return false;

	return _key == _event.key.keysym.sym;
}

string Keyboard::TextInput(SDL_Event _event)
{
	if (_event.type != SDL_TEXTINPUT) return "";

	return string(_event.text.text);
}