#ifndef PLAYER_H
#define PLAYER_H

#include "StandardIncludes.h"
#include "Renderer.h"
#include "SpriteSheet.h"

class Player
{
public:
	Player();
	virtual ~Player();

	void Initialize();
	void Update(float _deltaTime);
	void Render(Renderer* _renderer);
	void HandleInput(SDL_Event _event);

private:
	SpriteSheet* m_sprite;
	Point m_position;

	float scale;
	
	// Movement
	float m_walkSpeed;
	float m_runSpeed;
	float m_veloX;
	float m_veloY;
	bool m_isRunning;
	bool m_shiftDown;
	bool m_facingRight;

	// Gravity
	float m_gravity;
	bool m_isGrounded;
	float m_groundY;

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

