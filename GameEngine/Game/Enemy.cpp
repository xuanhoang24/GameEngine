#include "../Game/Enemy.h"
#include "../Game/GameMap.h"
#include "../Graphics/Camera.h"
#include "../Core/Timing.h"

Enemy::Enemy()
{
	m_animLoader = nullptr;
	m_worldX = 0.0f;
	m_worldY = 0.0f;
	m_isActive = true;
	m_type = EnemyType::Ghost;
	m_moveSpeed = 50.0f;
	m_direction = 1.0f;
	m_leftBoundary = 0.0f;
	m_rightBoundary = 0.0f;
	m_gameMap = nullptr;
}

Enemy::~Enemy()
{
	if (m_animLoader)
	{
		delete m_animLoader;
		m_animLoader = nullptr;
	}
}

void Enemy::Initialize(float x, float y, EnemyType type)
{
}

void Enemy::Initialize(float x, float y, EnemyType type, float leftBoundary, float rightBoundary)
{
	m_worldX = x;
	m_worldY = y;
	m_leftBoundary = leftBoundary;
	m_rightBoundary = rightBoundary;
	m_isActive = true;
	m_type = type;
	
	m_animLoader = new AnimatedSpriteLoader();
	
	if (m_type == EnemyType::Ghost)
	{
		m_animLoader->LoadAnimation("idle", "../Assets/Textures/Enemy/ghost1_fly.png", 1, 6, 16, 16, 6, 10.0f);
		m_moveSpeed = 30.0f;
	}
	else if (m_type == EnemyType::Mushroom)
	{
		m_animLoader->LoadAnimation("idle", "../Assets/Textures/Enemy/mushroom-walk.png", 1, 10, 16, 16, 10, 10.0f);
		m_moveSpeed = 40.0f;
	}
	
	// Random initial direction
	m_direction = (rand() % 2 == 0) ? -1.0f : 1.0f;
}

void Enemy::Update(float _deltaTime)
{
	if (!m_isActive)
		return;
	
	// Move in current direction
	m_worldX += m_moveSpeed * m_direction * _deltaTime;
	
	// Check if enemy has reached zone boundaries
	if (m_worldX >= m_rightBoundary)
	{
		m_direction = -1.0f; // Turn left
		m_worldX = m_rightBoundary; // Clamp position
	}
	else if (m_worldX <= m_leftBoundary)
	{
		m_direction = 1.0f; // Turn right
		m_worldX = m_leftBoundary; // Clamp position
	}
	
	// Check map collision
	if (m_gameMap)
	{
		float wallX;
		float width = GetWidth();
		float height = GetHeight();
		
		// Check collision based on direction
		if (m_direction > 0) // Moving right
		{
			if (m_gameMap->CheckCollisionLeft(m_worldX, m_worldY, width, height, wallX))
			{
				m_worldX = wallX - width; // Stop at wall
				m_direction = -1.0f; // Turn around
			}
		}
		else // Moving left
		{
			if (m_gameMap->CheckCollisionRight(m_worldX, m_worldY, width, height, wallX))
			{
				m_worldX = wallX; // Stop at wall
				m_direction = 1.0f; // Turn around
			}
		}
	}
}

vector<Enemy*> Enemy::SpawnEnemiesFromMap(GameMap* _map)
{
	vector<Enemy*> enemies;
	
	if (!_map)
		return enemies;
	
	srand((unsigned int)time(nullptr));
	
	// Get all enemy spawn points from the map
	const vector<pair<float, float>>& spawnPoints = _map->GetEnemySpawnPoints();
	
	// Create an enemy at each spawn point with random type
	for (const auto& spawn : spawnPoints)
	{
		Enemy* enemy = new Enemy();
		
		// Randomly choose between Ghost and Mushroom
		EnemyType randomType = (rand() % 2 == 0) ? EnemyType::Ghost : EnemyType::Mushroom;
		
		// Get zone boundaries for this spawn point
		float leftX = 0.0f;
		float rightX = 0.0f;
		_map->GetEnemyZoneBoundaries(spawn.first, spawn.second, leftX, rightX);
		
		enemy->Initialize(spawn.first, spawn.second, randomType, leftX, rightX);
		enemies.push_back(enemy);
	}
	
	return enemies;
}

void Enemy::Render(Renderer* _renderer, Camera* _camera)
{
	if (!m_isActive)
		return;
	
	float width = GetWidth();
	float height = GetHeight();
	
	// Get map width for looping
	int mapPixelWidth = 1600; // 100 tiles * 16 pixels (from map)
	
	// Get screen width to determine how many map copies to render
	Point screenSize = _renderer->GetWindowSize();
	int screenWidth = screenSize.X;
	
	float cameraX = _camera ? _camera->GetX() : 0.0f;
	
	// Calculate which map instances we need to render enemies for
	int startMapIndex = (int)floor((cameraX - width) / mapPixelWidth);
	int endMapIndex = (int)ceil((cameraX + screenWidth) / mapPixelWidth);
	
	// Render enemy at each map instance
	for (int mapIndex = startMapIndex; mapIndex <= endMapIndex; ++mapIndex)
	{
		float mapOffsetX = mapIndex * mapPixelWidth;
		float worldX = m_worldX + mapOffsetX;
		
		// Convert world position to screen position using camera
		float screenX = _camera ? _camera->WorldToScreenX(worldX) : worldX;
		float screenY = m_worldY;
		
		// Destination on the screen
		Rect destRect(
			(unsigned)screenX,
			(unsigned)screenY,
			(unsigned)(screenX + width),
			(unsigned)(screenY + height));
		
		string currentAnim = "idle";
		
		Rect srcRect = m_animLoader->UpdateAnimation(currentAnim, Timing::Instance().GetDeltaTime());
		Texture* currentTexture = m_animLoader->GetTexture(currentAnim);
		
		if (currentTexture)
			_renderer->RenderTexture(currentTexture, srcRect, destRect);
	}
}
