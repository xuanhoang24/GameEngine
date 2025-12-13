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
	scale = 2.0f;

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

	// Move player
	m_position.X += m_veloX * _deltaTime;

	// Gravity
	m_veloY += m_gravity * _deltaTime;

	// Apply velocity
	m_position.Y += (m_veloY * _deltaTime);
#pragma endregion Calculate Movement

#pragma region Animation Logic
	if (m_veloX == 0)
		m_sprite->Update(EN_AN_IDLE, _deltaTime);
	else if (m_isRunning)
		m_sprite->Update(EN_AN_RUN, _deltaTime);
	else
		m_sprite->Update(EN_AN_IDLE, _deltaTime);
#pragma endregion Animation Logic

	// Ground Collision with collision objects
	float width = GetWidth();
	float height = GetHeight();

	float footX = m_position.X + width * 0.5f;
	float groundY = 0.0f;

	bool foundGround = m_gameMap->CheckGround(
		footX - 1.0f,
		m_position.Y,
		2.0f,
		height,
		groundY
	);

	float prevBottom = m_prevY + height;
	float currBottom = m_position.Y + height;

	const float SNAP_EPS = 2.0f;

	if (foundGround && m_veloY >= 0 &&
		prevBottom <= groundY + SNAP_EPS &&
		currBottom >= groundY - SNAP_EPS)
	{
		m_position.Y = groundY - height;
		m_veloY = 0.0f;
		m_isGrounded = true;
		m_isJumping = false;
	}
	else
	{
		m_isGrounded = false;
	}

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

void Player::Render(Renderer* _renderer)
{
	float width = 69 * scale;
	float height = 44 * scale;

	float x = m_position.X;
	float y = m_position.Y;

	// Destination on the screen
	Rect destRect(
		(unsigned)x,
		(unsigned)y,
		(unsigned)(x + width),
		(unsigned)(y + height));

	if (!m_facingRight)
	{
		destRect = Rect(
			(unsigned)(x + width),
			(unsigned)y,
			(unsigned)x,
			(unsigned)(y + height)
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