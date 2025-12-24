#include "../Game/Player.h"
#include "../Game/GameMap.h"
#include "../Resources/FileController.h"
#include "../Core/Timing.h"
#include "../Input/InputController.h"
#include "../Input/Keyboard.h"

Player::Player()
{
	m_sprite = nullptr;
	m_position = Point(100, 100);
	m_worldX = 100.0f;
	scale = 1.0f;

	// Movement
	m_walkSpeed = 800.0f;
	m_runSpeed = 900.0f;
	m_veloX = 0;
	m_veloY = 0;
	m_isRunning = false;
	m_shiftDown = false;
	m_facingRight = true;
	m_prevY = m_position.Y;

	// Gravity
	m_gravity = 980.0f;
	m_groundY = 500.0f;
	m_isGrounded = false;

	// Map
	m_gameMap = nullptr;

	// Jump
	m_jumpPressed = false;
	m_isJumping = false;
	m_jumpForce = -350.0f;
	m_jumpHoldForce = -250.0f;
	m_jumpHoldTimer = 0.0f;
	m_jumpMaxHoldTime = 0.2f;
	m_coyoteTime = 0.12f;
	m_coyoteTimer = 0.0f;
}

Player::~Player()
{
}

void Player::Initialize()
{
	m_sprite = SpriteSheet::Pool->GetResource();
	m_sprite->Load("../Assets/Textures/Warrior.tga");
	m_sprite->SetSize(17, 6, 69, 44);
	m_sprite->AddAnimation(EN_AN_IDLE, 0, 6, 6.0f);
	m_sprite->AddAnimation(EN_AN_RUN, 6, 8, 6.0f);
	m_sprite->AddAnimation(EN_AN_JUMP_UP_FALL, 42, 8, 6.0f);
}

void Player::Update(float _deltaTime)
{
#pragma region Calculate Movement
	m_prevY = m_position.Y;

	// Move player in world space
	m_worldX += m_veloX * _deltaTime;

	// Gravity
	m_veloY += m_gravity * _deltaTime;

	// Apply velocity
	m_position.Y += (m_veloY * _deltaTime);
#pragma endregion Calculate Movement

#pragma region Collision Detection
	// Get collision box dimensions
	float collisionX, collisionY, collisionWidth, collisionHeight;
	GetCollisionBox(collisionX, collisionY, collisionWidth, collisionHeight);

	// Check ground collision using the collision box
	float groundY = 0.0f;
	bool foundGround = m_gameMap->CheckGround(
		collisionX,
		collisionY,
		collisionWidth,
		collisionHeight,
		groundY
	);

	float prevBottom = m_prevY + GetHeight();
	float currBottom = m_position.Y + GetHeight();

	const float SNAP_EPS = 2.0f;
	const float FALLBACK_GROUND = 200.0f;

	if (foundGround && m_veloY >= 0 &&
		prevBottom <= groundY + SNAP_EPS &&
		currBottom >= groundY - SNAP_EPS)
	{
		// Calculate offset from collision box bottom to sprite bottom
		float spriteHeight = GetHeight();
		float collisionOffsetY = spriteHeight - collisionHeight;
		
		// Position sprite so collision box sits on ground
		m_position.Y = groundY - collisionHeight - collisionOffsetY;
		m_veloY = 0.0f;
		m_isGrounded = true;
		m_isJumping = false;
	}
	else if (!foundGround && m_position.Y + GetHeight() >= FALLBACK_GROUND)
	{
		// Fallback ground if no collision objects found
		m_position.Y = FALLBACK_GROUND - GetHeight();
		m_veloY = 0.0f;
		m_isGrounded = true;
		m_isJumping = false;
	}
	else
	{
		m_isGrounded = false;
	}
#pragma endregion Collision Detection

#pragma region Animation Logic
	if (m_veloX == 0)
		m_sprite->Update(EN_AN_IDLE, _deltaTime);
	else if (m_isRunning)
		m_sprite->Update(EN_AN_RUN, _deltaTime);
	else
		m_sprite->Update(EN_AN_IDLE, _deltaTime);
#pragma endregion Animation Logic

	// Update Coyote Timer
	if (m_isGrounded)
	{
		// Reset coyote timer
		m_coyoteTimer = m_coyoteTime;
	}
	else
	{
		// Countdown while in air
		m_coyoteTimer -= _deltaTime;
	}

	// Jump
	if (m_jumpPressed)
	{
		if (!m_isJumping && (m_isGrounded || m_coyoteTimer > 0))
		{
			// Initial Jump
			m_isJumping = true;
			m_isGrounded = false;
			m_veloY = m_jumpForce;
			m_jumpHoldTimer = m_jumpMaxHoldTime;
		}

		// Hold jump
		if (m_isJumping && m_jumpHoldTimer > 0)
		{
			m_veloY += m_jumpHoldForce * _deltaTime;
			m_jumpHoldTimer -= _deltaTime;
		}
	}
}

void Player::Render(Renderer* _renderer, Camera* _camera)
{
	float width = 69 * scale;
	float height = 44 * scale;

	// Convert world position to screen position using camera
	float screenX = _camera ? _camera->WorldToScreenX(m_worldX) : m_worldX;
	float screenY = m_position.Y;

	// Destination on the screen
	Rect destRect(
		(unsigned)screenX,
		(unsigned)screenY,
		(unsigned)(screenX + width),
		(unsigned)(screenY + height));

	if (!m_facingRight)
	{
		destRect = Rect(
			(unsigned)(screenX + width),
			(unsigned)screenY,
			(unsigned)screenX,
			(unsigned)(screenY + height)
		);
	}

	// Get the part of the sprite sheet for the current animation frame
	Rect srcRect(0, 0, 0, 0);

	if (!m_isGrounded)
		srcRect = m_sprite->Update(EN_AN_JUMP_UP_FALL, Timing::Instance().GetDeltaTime());
	else if (m_veloX == 0)
		srcRect = m_sprite->Update(EN_AN_IDLE, Timing::Instance().GetDeltaTime());
	else if (m_isRunning)
		srcRect = m_sprite->Update(EN_AN_RUN, Timing::Instance().GetDeltaTime());
	else
		srcRect = m_sprite->Update(EN_AN_IDLE, Timing::Instance().GetDeltaTime());

	_renderer->RenderTexture(m_sprite, srcRect, destRect);
}

void Player::RenderCollisionBox(Renderer* _renderer, Camera* _camera)
{
	float spriteWidth = GetWidth();
	float spriteHeight = GetHeight();

	float collisionWidth = spriteWidth * 0.3f;
	float collisionHeight = spriteHeight * 0.80f;
	
	float horizontalOffset = m_facingRight ? -8.0f : 8.0f;
	float collisionX = m_worldX + (spriteWidth - collisionWidth) * 0.5f + horizontalOffset;
	
	// Align to feet
	float collisionY = m_position.Y + (spriteHeight - collisionHeight);

	// Convert world position to screen position using camera
	float screenX = _camera ? _camera->WorldToScreenX(collisionX) : collisionX;
	float screenY = collisionY;

	SDL_Renderer* sdl = _renderer->GetRenderer();

	// Set green color for player collision box
	SDL_SetRenderDrawColor(sdl, 0, 255, 0, 255);

	// Draw the collision box outline
	SDL_Rect collisionRect;
	collisionRect.x = (int)screenX;
	collisionRect.y = (int)screenY;
	collisionRect.w = (int)collisionWidth;
	collisionRect.h = (int)collisionHeight;

	SDL_RenderDrawRect(sdl, &collisionRect);
}

void Player::GetCollisionBox(float& outX, float& outY, float& outWidth, float& outHeight) const
{
	float spriteWidth = GetWidth();
	float spriteHeight = GetHeight();

	// Collision box dimensions (same as visual debug box)
	outWidth = spriteWidth * 0.3f;
	outHeight = spriteHeight * 0.80f;
	
	float horizontalOffset = m_facingRight ? -8.0f : 8.0f;
	outX = m_worldX + (spriteWidth - outWidth) * 0.5f + horizontalOffset;
	outY = m_position.Y + (spriteHeight - outHeight);
}

void Player::HandleInput(SDL_Event _event)
{
	Keyboard* kb = InputController::Instance().KB();
	const Uint8* keyState = SDL_GetKeyboardState(NULL);
	float speed = m_shiftDown ? m_runSpeed : m_walkSpeed;

	// SHIFT DOWN
	m_shiftDown = keyState[SDL_SCANCODE_LSHIFT] || keyState[SDL_SCANCODE_RSHIFT];
	// A Key
	if (keyState[SDL_SCANCODE_A])
	{
		m_veloX = -speed;
		m_isRunning = m_shiftDown;
		m_facingRight = false;
	}
	// D Key
	else if (keyState[SDL_SCANCODE_D])
	{
		m_veloX = speed;
		m_isRunning = m_shiftDown;
		m_facingRight = true;
	}
	else // Release A or D
	{
		m_veloX = 0;
		m_isRunning = false;
	}

	// Space key
	if (keyState[SDL_SCANCODE_SPACE])
	{
		m_jumpPressed = true;
	}
	else
	{
		m_jumpPressed = false;
	}
}