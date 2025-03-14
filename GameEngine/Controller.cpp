#include "Controller.h"

Controller::Controller()
{
}

Controller::~Controller()
{
}

void Controller::DetectControllers()
{
	m_controllers.clear();
	int numControllers = SDL_NumJoysticks();
	if (numControllers == 0) return;

	for (int count = 0; count < numControllers; count++)
	{
		if (!SDL_IsGameController(count)) continue;
		Add(SDL_GameControllerOpen(count), count);
	}
}

void Controller::Add(SDL_GameController* _controller, int _controllerID)
{
	if (_controller == nullptr) return;

	SDL_Joystick* j = SDL_GameControllerGetJoystick(_controller);
	SDL_JoystickID jID = SDL_JoystickInstanceID(j); // _controllerID is not to be used as a unique ID
	for (ControllerInfo c : m_controllers)
	{
		if (c.ID == jID) return;
	}

	ControllerInfo c = ControllerInfo();
	c.Controller = _controller;
	c.Name = string(SDL_GameControllerName(_controller));
	c.ID = jID;
	m_controllers.push_back(c);
}

bool Controller::Added(SDL_Event _event)
{
	if (_event.type != SDL_CONTROLLERDEVICEADDED) return false;

	if (SDL_IsGameController(_event.cdevice.which))
	{
		SDL_GameController* controller = SDL_GameControllerOpen(_event.cdevice.which);
		Add(controller, _event.cdevice.which);
		
		return true;
	}

	return false;
}
bool Controller::Removed(SDL_Event _event)
{
	if (_event.type != SDL_CONTROLLERDEVICEREMOVED) return false;

	for (unsigned int count = 0; count < m_controllers.size(); count++)
	{
		if (m_controllers[count].ID != _event.cdevice.which) continue;
		m_controllers.erase(m_controllers.begin() + count);

		return true;
	}

	return false;
}

bool Controller::ProcessButtons(SDL_Event _event)
{
	if (_event.type == SDL_CONTROLLERBUTTONDOWN)
	{
		for (unsigned int count = 0; count < m_controllers.size(); count++)
		{
			if (m_controllers[count].ID != _event.cdevice.which) continue;
			auto v = m_controllers[count].Buttons;
			if (std::find(v.begin(), v.end(), _event.cbutton.button) == v.end())
			{
				m_controllers[count].Buttons.push_back(SDL_GameControllerButton(_event.cbutton.button));
			}
			break;
		}
		return true;
	}
	else if (_event.type == SDL_CONTROLLERBUTTONUP)
	{
		for (unsigned int count = 0; count < m_controllers.size(); count++)
		{
			if (m_controllers[count].ID != _event.cdevice.which) continue;
			vector<SDL_GameControllerButton>* v = &m_controllers[count].Buttons;
			for (unsigned int button = 0; button < v->size(); button++)
			{
				if ((*v)[button] == _event.cbutton.button)
				{
					v->erase(v->begin() + button);
					break;
				}
			}
		}
		return true;
	}

	return false;
}

string Controller::ToString()
{
	int cc = 0;
	string s = "Controllers: ";
	for (ControllerInfo c : m_controllers)
	{
		cc++;
		s += to_string(cc) + " - " + c.Name + ": ";
		s += c.ToString();
	}
	return s;
}

bool Controller::ProcessMotion(SDL_Event _event)
{
	if (_event.type != SDL_CONTROLLERAXISMOTION) return false;

	for (unsigned int count = 0; count < m_controllers.size(); count++)
	{
		if (m_controllers[count].ID != _event.cdevice.which) continue;

		// Left Analog Stick
		if (_event.caxis.axis == 0) // X axis motion
		{
			m_controllers[count].LeftAxis.X = _event.caxis.value;
		}
		else if (_event.caxis.axis == 1) // Y axis motion
		{
			m_controllers[count].LeftAxis.Y = _event.caxis.value;
		}

		// Right Analog Stick
		if (_event.caxis.axis == 2) // X axis motion
		{
			m_controllers[count].RightAxis.X = _event.caxis.value;
		}
		else if (_event.caxis.axis == 3) // Y axis motion
		{
			m_controllers[count].RightAxis.Y = _event.caxis.value;
		}
	}
	return true;
}