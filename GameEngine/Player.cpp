#include "Player.h"
#include "FileController.h"
#include "Timing.h"

Player::Player()
{
	m_sprite = nullptr;
	m_position = Point(100, 300);
	scale = 2.0f;
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
}

void Player::Update(float _deltaTime)
{
	// Update the animation frame using delta time
	m_sprite->Update(EN_AN_IDLE, _deltaTime);
}

void Player::Render(Renderer* _renderer)
{
	float width = 69 * scale;
	float height = 44 * scale;

	float x = m_position.X;
	float y = m_position.Y;

	// Destination on the screen
	Rect destRect( (unsigned)x, (unsigned)y,
		(unsigned)(x + width), (unsigned)(y + height));

	// Get the part of the sprite sheet for the current animation frame
	Rect srcRect = m_sprite->Update(EN_AN_IDLE, 0);

	_renderer->RenderTexture(m_sprite, srcRect, destRect);

}