#include "InputController.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Controller.h"

InputController::InputController()
{
	m_keyboard = new Keyboard();
	m_mouse = new Mouse();
	m_controller = new Controller();
}

InputController::~InputController()
{
	if (m_keyboard != nullptr)
	{
		delete m_keyboard;
		m_keyboard = nullptr;
	}

	if (m_mouse != nullptr)
	{
		delete m_mouse;
		m_mouse = nullptr;
	}

	if (m_controller != nullptr)
	{
		delete m_controller;
		m_controller = nullptr;
	}
}