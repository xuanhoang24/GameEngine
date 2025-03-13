#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "StandardIncludes.h"

class Keyboard
{
public:
	// Constructors/Destructors
	Keyboard();
	virtual ~Keyboard();

	// Methods
	bool KeyDown(SDL_Event _event, SDL_Keycode _key);
	bool KeyUp(SDL_Event _event, SDL_Keycode _key);
	string TextInput(SDL_Event _event);

};

#endif // KEYBOARD_H

