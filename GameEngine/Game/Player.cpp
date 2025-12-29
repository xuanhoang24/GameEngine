#include "../Game/Player.h"
#include "../Game/ChunkMap.h"
#include "../Resources/FileController.h"
#include "../Core/Timing.h"
#include "../Input/InputController.h"
#include "../Input/Keyboard.h"

Player::Player()
{
	m_animLoader = nullptr;
	m_worldX = 20.0f;
	m_posY = 50.0f;
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
	m_chunkMap = nullptr;

	m_jumpPressed = false;
	m_isJumping = false;
	m_jumpForce = -400.0f;
	m_jumpHoldForce = -300.0f;
	m_jumpHoldTimer = 0.0f;
	m_jumpMaxHoldTime = 0.2f;
	m_coyoteTime = 0.12f;
	m_coyoteTimer = 0.0f;

	m_isDead = false;
	m_isFullyDead = false;
	m_deathTimer = 0.0f;
	m_deathDuration = 1.0f; // Show hurt animation for 1 second
}

Player::~Player()
{
	if (m_animLoader)
	{
		delete m_animLoader;
		m_animLoader = nullptr;
	}
}

void Player::Initialize()
{
	m_animLoader = new AnimatedSpriteLoader();

	m_animLoader->LoadAnimation("idle", "../Assets/Textures/Player/idle.png", 1, 4, 16, 16, 4, 8.0f);

	m_animLoader->LoadAnimation("run", "../Assets/Textures/Player/run.png", 1, 4, 16, 16, 4, 12.0f);

	m_animLoader->LoadAnimation("jumpandfall", "../Assets/Textures/Player/jumpandfall.png", 1, 2, 16, 16, 2, 8.0f);

	m_animLoader->LoadAnimation("hurt", "../Assets/Textures/Player/hurt.png", 1, 2, 16, 16, 2, 2.0f);
}

void Player::Update(float _deltaTime)
{
	// Handle death animation
	if (m_isDead)
	{
		m_deathTimer += _deltaTime;
		if (m_deathTimer >= m_deathDuration)
		{
			m_isFullyDead = true;
		}
		return;
	}

	// Clamp deltaTime to prevent huge jumps during loading
	if (_deltaTime > 0.033f)
		_deltaTime = 0.033f;

	// Apply movement
	m_worldX += m_veloX * _deltaTime;
	m_veloY += m_gravity * _deltaTime;
	m_posY += m_veloY * _deltaTime;

	// Check if player fell below death threshold (only die when falling below map)
	if (m_posY > 260.0f)
	{
		Die();
		return;
	}

	// Allow player to jump freely above the map - gravity will bring them back down

	// Collision detection
	if (!m_chunkMap)
	{
		const float FALLBACK_GROUND = 500.0f;
		if (m_posY + GetHeight() >= FALLBACK_GROUND)
		{
			m_posY = FALLBACK_GROUND - GetHeight();
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
			if (m_chunkMap->CheckCollisionLeft(collisionX, collisionY, collisionWidth, collisionHeight, wallX))
			{
				float offsetX = (GetWidth() - collisionWidth) * 0.5f;
				m_worldX = wallX - collisionWidth - offsetX;
				m_veloX = 0;
			}
		}
		else if (m_veloX < 0)
		{
			float wallX;
			if (m_chunkMap->CheckCollisionRight(collisionX, collisionY, collisionWidth, collisionHeight, wallX))
			{
				float offsetX = (GetWidth() - collisionWidth) * 0.5f;
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
			if (m_chunkMap->CheckCollisionTop(collisionX, collisionY, collisionWidth, collisionHeight, groundY))
			{
				float offsetY = GetHeight() - collisionHeight;
				// Snap player to ground - handles both landing and passing through
				m_posY = groundY - collisionHeight - offsetY;
				m_veloY = 0.0f;
				m_isGrounded = true;
				m_isJumping = false;
			}
			else
			{
				const float FALLBACK_GROUND = 500.0f;
				if (m_posY + GetHeight() >= FALLBACK_GROUND)
				{
					m_posY = FALLBACK_GROUND - GetHeight();
					m_veloY = 0.0f;
					m_isGrounded = true;
					m_isJumping = false;
				}
			}
		}
		else if (m_veloY < 0)
		{
			float ceilingY;
			if (m_chunkMap->CheckCollisionBottom(collisionX, collisionY, collisionWidth, collisionHeight, ceilingY))
			{
				float offsetY = GetHeight() - collisionHeight;
				m_posY = ceilingY - offsetY;
				m_veloY = 0.0f;
				m_jumpHoldTimer = 0.0f;
			}
		}
	}

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
	if (m_isFullyDead)
		return;

	float width = 16 * scale;
	float height = 16 * scale;

	// Convert world position to screen position using camera
	float screenX = _camera ? _camera->WorldToScreenX(m_worldX) : m_worldX;
	float screenY = m_posY;

	// Don't render if player is above the screen
	if (screenY + height < 0)
		return;

	// Destination on the screen
	Rect destRect(
		(unsigned)screenX,
		(unsigned)(screenY < 0 ? 0 : screenY),
		(unsigned)(screenX + width),
		(unsigned)(screenY + height));

	// Flip sprite horizontally when facing left
	if (!m_facingRight)
	{
		destRect = Rect(
			(unsigned)(screenX + width),
			(unsigned)(screenY < 0 ? 0 : screenY),
			(unsigned)screenX,
			(unsigned)(screenY + height));
	}

	string currentAnim = "idle";
	if (m_isDead)
		currentAnim = "hurt";
	else if (!m_isGrounded)
		currentAnim = "jumpandfall";
	else if (m_veloX != 0)
		currentAnim = "run";

	Rect srcRect = m_animLoader->UpdateAnimation(currentAnim, Timing::Instance().GetDeltaTime());
	Texture* currentTexture = m_animLoader->GetTexture(currentAnim);

	if (currentTexture)
		_renderer->RenderTexture(currentTexture, srcRect, destRect);
}

void Player::RenderCollisionBox(Renderer* _renderer, Camera* _camera)
{
	float collisionWidth = 16.0f * scale;
	float collisionHeight = 16.0f * scale;
	float collisionX = m_worldX + (GetWidth() - collisionWidth) * 0.5f;
	float collisionY = m_posY + GetHeight() - collisionHeight;

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
	// x,y is the top-left of the spawn point in Tiled
	// Center the sprite horizontally on the spawn point
	m_worldX = x;
	// Position player at the spawn Y (top-left of spawn object)
	m_posY = y;
	m_veloY = 0.0f;
	m_veloX = 0.0f;
	m_isGrounded = false;
	m_isJumping = false;
}

void Player::GetCollisionBox(float& outX, float& outY, float& outWidth, float& outHeight) const
{
	outWidth = 16.0f * scale;
	outHeight = 16.0f * scale;
	outX = m_worldX + (GetWidth() - outWidth) * 0.5f;
	outY = m_posY + GetHeight() - outHeight;
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

void Player::Die()
{
	if (!m_isDead)
	{
		m_isDead = true;
		m_deathTimer = 0.0f;
		m_veloX = 0.0f;
		m_veloY = 0.0f;
	}
}


void Player::Reset()
{
	m_isDead = false;
	m_isFullyDead = false;
	m_deathTimer = 0.0f;
	m_veloX = 0.0f;
	m_veloY = 0.0f;
	m_isGrounded = false;
	m_isJumping = false;
	m_jumpPressed = false;
	m_facingRight = true;
}
