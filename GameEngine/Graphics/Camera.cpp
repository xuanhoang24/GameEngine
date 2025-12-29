#include "../Graphics/Camera.h"
#include "../Game/Player.h"
#include "../Graphics/Renderer.h"

Camera::Camera()
{
	m_x = 0.0f;
	m_y = 0.0f;
	m_targetPlayer = nullptr;
	m_renderer = nullptr;
	m_maxX = 0.0f; // Track furthest camera position
}

Camera::~Camera()
{
}

void Camera::Update(float _deltaTime)
{
	if (m_targetPlayer && m_renderer)
	{
		FollowPlayer(m_targetPlayer, m_renderer);
	}
}

void Camera::FollowPlayer(Player* _player, Renderer* _renderer)
{
	if (!_player || !_renderer)
		return;

	m_targetPlayer = _player;
	m_renderer = _renderer;

	float playerWorldX = _player->GetWorldX();
	float playerWidth = _player->GetWidth();
	
	// Get logical size from renderer (matches screen)
	Point logicalSize = _renderer->GetLogicalSize();
	int logicalWidth = logicalSize.X;
	int logicalHeight = logicalSize.Y;

	// Center camera on player horizontally
	float targetX = playerWorldX + playerWidth * 0.5f - logicalWidth * 0.5f;

	// Don't let camera go negative at the start
	if (targetX < 0)
		targetX = 0;

	// Prevent camera from moving backward
	if (targetX > m_maxX)
	{
		m_maxX = targetX;
		m_x = targetX;
	}
	else
	{
		m_x = m_maxX; // Keep camera at furthest position
	}
	
	// Keep camera at y=0 to show full map height
	m_y = 0;
}

void Camera::SetPosition(float _x, float _y)
{
	m_x = _x;
	m_y = _y;
}

float Camera::WorldToScreenX(float _worldX) const
{
	return _worldX - m_x;
}

float Camera::WorldToScreenY(float _worldY) const
{
	return _worldY - m_y;
}

float Camera::ScreenToWorldX(float _screenX) const
{
	return _screenX + m_x;
}

float Camera::ScreenToWorldY(float _screenY) const
{
	return _screenY + m_y;
}

void Camera::Reset()
{
	m_x = 0.0f;
	m_y = 0.0f;
	m_maxX = 0.0f;
}
