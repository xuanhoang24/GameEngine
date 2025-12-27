#include "../Game/Coin.h"
#include "../Game/GameMap.h"
#include "../Graphics/Camera.h"
#include "../Core/Timing.h"

Coin::Coin()
{
	m_animLoader = nullptr;
	m_worldX = 0.0f;
	m_worldY = 0.0f;
	m_isActive = true;
	m_collectedInMapLoop = -1; // -1 means never collected
	m_lastCameraX = 0.0f;
}

Coin::~Coin()
{
	if (m_animLoader)
	{
		delete m_animLoader;
		m_animLoader = nullptr;
	}
}

void Coin::Initialize(float x, float y)
{
	// Center the sprite horizontally on the spawn point
	m_worldX = x - (GetWidth() * 0.5f);
	// Adjust Y so the bottom of the sprite is at the spawn point
	m_worldY = y - GetHeight();
	m_isActive = true;
	
	m_animLoader = new AnimatedSpriteLoader();
	m_animLoader->LoadAnimation("idle", "../Assets/Textures/Obstacles/coin2.png", 1, 10, 16, 16, 10, 10.0f);
}

void Coin::Update(float _deltaTime)
{
}

void Coin::CheckRespawn(float _cameraX, int _mapPixelWidth)
{
	// Initialize last camera position on first check
	if (m_lastCameraX == 0.0f)
	{
		m_lastCameraX = _cameraX;
		return;
	}
	
	// Add buffer to respawn entities before entering the next map
	float respawnBuffer = 400.0f; // Respawn 400 pixels before next map starts
	
	// Calculate which map loop the camera is currently in (with buffer for lookahead)
	int currentMapLoop = (int)floor((_cameraX + respawnBuffer) / _mapPixelWidth);
	int lastMapLoop = (int)floor((m_lastCameraX + respawnBuffer) / _mapPixelWidth);
	
	// If entered a new map loop (with buffer)
	if (currentMapLoop > lastMapLoop)
	{
		// Always respawn when entering a new map loop
		m_isActive = true;
		m_collectedInMapLoop = -1; // Reset collected status for new loop
		
		m_lastCameraX = _cameraX;
	}
	else if (!m_isActive && m_collectedInMapLoop == -1)
	{
		// Track when coin is collected in current loop (without buffer)
		int actualCurrentLoop = (int)floor(_cameraX / _mapPixelWidth);
		m_collectedInMapLoop = actualCurrentLoop;
	}
}

vector<Coin*> Coin::SpawnCoinsFromMap(GameMap* _map)
{
	vector<Coin*> coins;
	
	if (!_map)
		return coins;
	
	// Get all coin spawn points from the map
	const vector<pair<float, float>>& spawnPoints = _map->GetCoinSpawnPoints();
	
	// Create a coin at each spawn point
	for (const auto& spawn : spawnPoints)
	{
		Coin* coin = new Coin();
		coin->Initialize(spawn.first, spawn.second);
		coins.push_back(coin);
	}
	
	return coins;
}

void Coin::Render(Renderer* _renderer, Camera* _camera)
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
	
	// Calculate which map instances we need to render coins for
	int startMapIndex = (int)floor((cameraX - width) / mapPixelWidth);
	int endMapIndex = (int)ceil((cameraX + screenWidth) / mapPixelWidth);
	
	// Render coin at each map instance
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
