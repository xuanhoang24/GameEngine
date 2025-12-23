#ifndef CAMERA_H
#define CAMERA_H

#include "../Core/StandardIncludes.h"

class Player;
class Renderer;

class Camera
{
public:
	Camera();
	virtual ~Camera();

	// Update camera position to follow target
	void Update(float _deltaTime);
	void FollowPlayer(Player* _player, Renderer* _renderer);

	// Getters
	float GetX() const { return m_x; }
	float GetY() const { return m_y; }

	// Setters
	void SetX(float _x) { m_x = _x; }
	void SetY(float _y) { m_y = _y; }
	void SetPosition(float _x, float _y);

	// World to screen conversion
	float WorldToScreenX(float _worldX) const;
	float WorldToScreenY(float _worldY) const;
	float ScreenToWorldX(float _screenX) const;
	float ScreenToWorldY(float _screenY) const;

private:
	float m_x;  // Camera X position in world space
	float m_y;  // Camera Y position in world space
	
	Player* m_targetPlayer;
	Renderer* m_renderer;
};

#endif // CAMERA_H
