#include "../Game/Enemy.h"
#include "../Graphics/Camera.h"

Enemy::Enemy()
{
	m_type = EnemyType::Ghost;
	m_moveSpeed = 50.0f;
	m_direction = 1.0f;
	m_baseLeftBoundary = 0.0f;
	m_baseRightBoundary = 0.0f;
}

Enemy::~Enemy()
{
}

void Enemy::Initialize(float x, float y)
{
	Initialize(x, y, EnemyType::Ghost, x - 50.0f, x + 50.0f);
}

void Enemy::Initialize(float x, float y, EnemyType type, float leftBoundary, float rightBoundary)
{
	SetBasePosition(x, y);
	
	m_baseLeftBoundary = leftBoundary;
	m_baseRightBoundary = rightBoundary;
	
	m_isActive = true;
	m_currentMapInstance = 0;
	m_type = type;
	
	RandomEnemy();
	
	m_direction = (rand() % 2 == 0) ? -1.0f : 1.0f;
}

void Enemy::Update(float _deltaTime, float _cameraX, int _screenWidth, int _mapPixelWidth)
{
	// Call base class update for repositioning logic
	WorldEntity::Update(_deltaTime, _cameraX, _screenWidth, _mapPixelWidth);
	
	if (!m_isActive)
		return;
	
	// Calculate current boundaries based on map instance
	float mapOffset = m_currentMapInstance * _mapPixelWidth;
	float leftBound = m_baseLeftBoundary + mapOffset;
	float rightBound = m_baseRightBoundary + mapOffset;
	
	// Move in current direction
	m_worldX += m_moveSpeed * m_direction * _deltaTime;
	
	// Check if enemy has reached zone boundaries
	if (m_worldX >= rightBound)
	{
		m_direction = -1.0f;
		m_worldX = rightBound;
	}
	else if (m_worldX <= leftBound)
	{
		m_direction = 1.0f;
		m_worldX = leftBound;
	}
}

void Enemy::OnRepositionAhead()
{
	m_direction = (rand() % 2 == 0) ? -1.0f : 1.0f;
}

void Enemy::Reset()
{
	WorldEntity::Reset();
	m_direction = (rand() % 2 == 0) ? -1.0f : 1.0f;
	
	// Randomize enemy type on reset
	EnemyType newType = (rand() % 2 == 0) ? EnemyType::Ghost : EnemyType::Mushroom;
	if (newType != m_type)
	{
		m_type = newType;
		RandomEnemy();
	}
}

void Enemy::RandomEnemy()
{
	if (m_animLoader)
	{
		delete m_animLoader;
		m_animLoader = nullptr;
	}
	
	m_animLoader = new AnimatedSpriteLoader();
	
	if (m_type == EnemyType::Ghost)
	{
		m_animLoader->LoadAnimation("idle", "../Assets/Textures/Enemy/ghost1_fly.png", 1, 6, 16, 16, 6, 10.0f);
		m_moveSpeed = 30.0f;
	}
	else
	{
		m_animLoader->LoadAnimation("idle", "../Assets/Textures/Enemy/mushroom-walk.png", 1, 10, 16, 16, 10, 10.0f);
		m_moveSpeed = 40.0f;
	}
}
