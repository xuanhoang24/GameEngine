#include "../Graphics/Camera.h"
#include "../Game/Player.h"
#include "../Graphics/Renderer.h"

Camera::Camera()
{
	m_x = 0.0f;
	m_y = 0.0f;
	m_targetPlayer = nullptr;
	m_renderer = nullptr;
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
	int screenWidth = _renderer->GetWindowSize().X;

	// Center camera on player
	float targetX = playerWorldX + playerWidth * 0.5f - screenWidth * 0.5f;

	// Don't let camera go negative at the start
	if (targetX < 0)
		targetX = 0;

	m_x = targetX;
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
