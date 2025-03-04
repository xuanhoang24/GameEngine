#include "Timing.h"

Timing::Timing()
{
	m_fpsCount = 0;
	m_fpsLast = 0;
	m_deltaTime = SDL_GetTicks();
	m_lastTime = m_currentTime;
	m_fpsStart = m_currentTime;
	SetFPS(60);
}

void Timing::Tick()
{
	m_currentTime = SDL_GetTicks();
	m_deltaTime = (float)(m_currentTime - m_lastTime) / 1000.0f;
	if (m_fpsStart + 1000 <= m_currentTime)
	{
		m_fpsLast = m_fpsCount;
		m_fpsCount = 0;
		m_fpsStart = m_currentTime;
	}
	else
	{
		m_fpsCount++;
	}
	m_lastTime = m_currentTime;
}

void Timing::SetFPS(int _fps)
{
	m_targetFPS = _fps;
	m_tickPerFrame = 1000 / m_targetFPS;
}

void Timing::CapFPS()
{
	unsigned int delta = SDL_GetTicks() - m_currentTime;
	if(delta <m_tickPerFrame)
	{
		SDL_Delay(m_tickPerFrame - delta);
	}
}