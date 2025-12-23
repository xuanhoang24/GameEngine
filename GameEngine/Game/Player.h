#ifndef PLAYER_H
#define PLAYER_H

#include "../Core/StandardIncludes.h"
#include "../Graphics/Renderer.h"
#include "../Graphics/SpriteSheet.h"

class GameMap;
class Camera;

class Player
{
public:
	Player();
	virtual ~Player();

	void Initialize();
	void Update(float _deltaTime);
	void Render(Renderer* _renderer, Camera* _camera);
	void HandleInput(SDL_Event _event);
	void SetGameMap(GameMap* _map) { m_gameMap = _map; }
	
	float GetWidth() const { return 69 * scale; }
	float GetHeight() const { return 44 * scale; }
	float GetWorldX() const { return m_worldX; }
	float GetWorldY() const { return m_position.Y; }

private:
	SpriteSheet* m_sprite;
	Point m_position; // Screen position
	float m_worldX;   // World position

	float scale;
	
	// Movement
	float m_walkSpeed;
	float m_runSpeed;
	float m_veloX;
	float m_veloY;
	bool m_isRunning;
	bool m_shiftDown;
	bool m_facingRight;
	float m_prevY;

	// Gravity
	float m_gravity;
	bool m_isGrounded;
	float m_groundY;
	
	// Map reference
	GameMap* m_gameMap;

	// Jump
	bool m_jumpPressed;
	bool m_isJumping;
	float m_jumpForce;
	float m_jumpHoldForce;
	float m_jumpMaxHoldTime;
	float m_jumpHoldTimer;
	float m_coyoteTime;
	float m_coyoteTimer;


};

#endif // PLAYER_H

