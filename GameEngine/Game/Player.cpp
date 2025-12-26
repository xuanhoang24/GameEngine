#include "../Game/Player.h"
#include "../Game/GameMap.h"
#include "../Resources/FileController.h"
#include "../Core/Timing.h"
#include "../Input/InputController.h"
#include "../Input/Keyboard.h"

Player::Player()
{
	m_sprite = nullptr;
	m_position = Point(100, 50);
	m_worldX = 20.0f;
	scale = 1.0f;

	m_walkSpeed = 150.0f;
	m_runSpeed = 250.0f;
	m_veloX = 0;
	m_veloY = 0;
	m_isRunning = false;
	m_shiftDown = false;
	m_facingRight = true;

	m_gravity = 980.0f;
	m_isGrounded = false;
	m_gameMap = nullptr;

	m_jumpPressed = false;
	m_isJumping = false;
	m_jumpForce = -400.0f;
	m_jumpHoldForce = -300.0f;
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
	// Clamp deltaTime to prevent huge jumps during loading
	if (_deltaTime > 0.033f)
		_deltaTime = 0.033f;

	// Apply movement
	m_worldX += m_veloX * _deltaTime;
	m_veloY += m_gravity * _deltaTime;
	m_position.Y += (unsigned int)(m_veloY * _deltaTime);

	// Prevent player from going above screen
	if ((int)m_position.Y < 0)
	{
		m_position.Y = 0;
		m_veloY = 0.0f;
	}

	// Collision detection
	if (!m_gameMap)
	{
		const float FALLBACK_GROUND = 500.0f;
		if (m_position.Y + GetHeight() >= FALLBACK_GROUND)
		{
			m_position.Y = (unsigned int)(FALLBACK_GROUND - GetHeight());
			m_veloY = 0.0f;
			m_isGrounded = true;
			m_isJumping = false;
		}
		else
			m_isGrounded = false;
	}
	else
	{
		float collisionX, collisionY, collisionWidth, collisionHeight;
		GetCollisionBox(collisionX, collisionY, collisionWidth, collisionHeight);

		// Horizontal collision
		if (m_veloX > 0)
		{
			float wallX;
			if (m_gameMap->CheckCollisionLeft(collisionX, collisionY, collisionWidth, collisionHeight, wallX))
			{
				float offsetX = (GetWidth() - collisionWidth) * 0.5f + (m_facingRight ? -8.0f : 8.0f);
				m_worldX = wallX - collisionWidth - offsetX;
				m_veloX = 0;
			}
		}
		else if (m_veloX < 0)
		{
			float wallX;
			if (m_gameMap->CheckCollisionRight(collisionX, collisionY, collisionWidth, collisionHeight, wallX))
			{
				float offsetX = (GetWidth() - collisionWidth) * 0.5f + (m_facingRight ? -8.0f : 8.0f);
				m_worldX = wallX - offsetX;
				m_veloX = 0;
			}
		}

		GetCollisionBox(collisionX, collisionY, collisionWidth, collisionHeight);
		m_isGrounded = false;

		// Vertical collision
		if (m_veloY >= 0)
		{
			float groundY;
			if (m_gameMap->CheckCollisionTop(collisionX, collisionY, collisionWidth, collisionHeight, groundY))
			{
				float offsetY = GetHeight() - collisionHeight;
				float distanceToGround = groundY - (collisionY + collisionHeight);
				
				if (distanceToGround <= 5.0f)
				{
					m_position.Y = (unsigned int)(groundY - collisionHeight - offsetY);
					m_veloY = 0.0f;
					m_isGrounded = true;
					m_isJumping = false;
				}
			}
			else
			{
				const float FALLBACK_GROUND = 500.0f;
				if (m_position.Y + GetHeight() >= FALLBACK_GROUND)
				{
					m_position.Y = (unsigned int)(FALLBACK_GROUND - GetHeight());
					m_veloY = 0.0f;
					m_isGrounded = true;
					m_isJumping = false;
				}
			}
		}
		else if (m_veloY < 0)
		{
			float ceilingY;
			if (m_gameMap->CheckCollisionBottom(collisionX, collisionY, collisionWidth, collisionHeight, ceilingY))
			{
				float offsetY = GetHeight() - collisionHeight;
				m_position.Y = (unsigned int)(ceilingY - offsetY);
				m_veloY = 0.0f;
				m_jumpHoldTimer = 0.0f;
			}
		}
	}

	// Animation
	if (m_veloX == 0)
		m_sprite->Update(EN_AN_IDLE, _deltaTime);
	else if (m_isRunning)
		m_sprite->Update(EN_AN_RUN, _deltaTime);
	else
		m_sprite->Update(EN_AN_IDLE, _deltaTime);

	// Jump logic
	m_coyoteTimer = m_isGrounded ? m_coyoteTime : m_coyoteTimer - _deltaTime;

	if (m_jumpPressed)
	{
		if (!m_isJumping && (m_isGrounded || m_coyoteTimer > 0))
		{
			m_isJumping = true;
			m_isGrounded = false;
			m_veloY = m_jumpForce;
			m_jumpHoldTimer = m_jumpMaxHoldTime;
		}

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

void Player::SetSpawnPosition(float x, float y)
{
	m_worldX = x;
	m_position.Y = (unsigned int)y;
	m_veloY = 0.0f;
	m_veloX = 0.0f;
	m_isGrounded = false;
	m_isJumping = false;
}

void Player::GetCollisionBox(float& outX, float& outY, float& outWidth, float& outHeight) const
{
	outWidth = GetWidth() * 0.3f;
	outHeight = GetHeight() * 0.80f;
	
	float horizontalOffset = m_facingRight ? -8.0f : 8.0f;
	outX = m_worldX + (GetWidth() - outWidth) * 0.5f + horizontalOffset;
	outY = m_position.Y + (GetHeight() - outHeight);
}

void Player::HandleInput(SDL_Event _event)
{
	const Uint8* keyState = SDL_GetKeyboardState(NULL);
	m_shiftDown = keyState[SDL_SCANCODE_LSHIFT] || keyState[SDL_SCANCODE_RSHIFT];
	float speed = m_shiftDown ? m_runSpeed : m_walkSpeed;

	if (keyState[SDL_SCANCODE_A])
	{
		m_veloX = -speed;
		m_isRunning = m_shiftDown;
		m_facingRight = false;
	}
	else if (keyState[SDL_SCANCODE_D])
	{
		m_veloX = speed;
		m_isRunning = m_shiftDown;
		m_facingRight = true;
	}
	else
	{
		m_veloX = 0;
		m_isRunning = false;
	}

	m_jumpPressed = keyState[SDL_SCANCODE_SPACE];
}