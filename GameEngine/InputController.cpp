#include "InputController.h"
#include "Keyboard.h"

InputController::InputController()
{
	m_keyboard = new Keyboard();
}

InputController::~InputController()
{
	if (m_keyboard != nullptr)
	{
		delete m_keyboard;
		m_keyboard = nullptr;
	}
}