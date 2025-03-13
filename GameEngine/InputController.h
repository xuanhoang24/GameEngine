#ifndef INPUTCONTROLLER_H
#define INPUTCONTROLLER_H

#include "StandardIncludes.h"

class Keyboard;
class Mouse;
class Controller;

class InputController : public Singleton<InputController>
{
public:
	// Constructors/Destructors
	InputController();
	virtual ~InputController();

	// Accessors
	Keyboard* KB() { return m_keyboard; }
	Mouse* MS() { return m_mouse; }
	Controller* CT() { return m_controller; }

private:
	Keyboard* m_keyboard;
	Mouse* m_mouse;
	Controller* m_controller;
};

#endif // INPUTCONTROLLER_H
