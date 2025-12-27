#include "../Game/Coin.h"
#include "../Game/GameMap.h"
#include "../Graphics/Camera.h"
#include "../Core/Timing.h"
#include <random>

static CoinTextureInfo GetRandomCoinTexture()
{
	static random_device rd;
	static mt19937 gen(rd());
	static uniform_int_distribution<> dist(0, 2);
	
	int coinType = dist(gen);
	switch (coinType)
	{
		case 0: return { "../Assets/Textures/Obstacles/coin1.png", 10 };
		case 1: return { "../Assets/Textures/Obstacles/coin2.png", 10 };
		case 2: return { "../Assets/Textures/Obstacles/diamond.png", 5 };
		default: return { "../Assets/Textures/Obstacles/coin1.png", 10 };
	}
}

Coin::Coin()
{
	m_animLoader = nullptr;
	m_worldX = 0.0f;
	m_worldY = 0.0f;
	m_baseX = 0.0f;
	m_baseY = 0.0f;
	m_isActive = true;
	m_currentMapInstance = 0;
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
	// Store base position within single map
	m_baseX = x - (GetWidth() * 0.5f);
	m_baseY = y - GetHeight();
	
	// Initial world position is same as base
	m_worldX = m_baseX;
	m_worldY = m_baseY;
	
	m_isActive = true;
	m_currentMapInstance = 0;
	
	m_animLoader = new AnimatedSpriteLoader();
	CoinTextureInfo texInfo = GetRandomCoinTexture();
	m_animLoader->LoadAnimation("idle", texInfo.path, 1, texInfo.frameCount, 16, 16, texInfo.frameCount, 10.0f);
}

void Coin::Update(float _deltaTime, float _cameraX, int _screenWidth, int _mapPixelWidth)
{
	// Check if coin went behind camera (left of screen) - reposition ahead
	float cameraLeftEdge = _cameraX;
	float coinRightEdge = m_worldX + GetWidth();
	
	if (coinRightEdge < cameraLeftEdge - 50.0f) // 50px buffer behind camera
	{
		RepositionAhead(_cameraX, _screenWidth, _mapPixelWidth);
	}
}

void Coin::RepositionAhead(float _cameraX, int _screenWidth, int _mapPixelWidth)
{
	// Calculate which map instance is ahead of camera (right side of screen + buffer)
	float aheadX = _cameraX + _screenWidth + 100.0f; // 100px buffer ahead
	int targetMapInstance = (int)floor(aheadX / _mapPixelWidth);
	
	// Make sure moving forward, not backward
	if (targetMapInstance <= m_currentMapInstance)
		targetMapInstance = m_currentMapInstance + 1;
	
	// Update to new map instance
	m_currentMapInstance = targetMapInstance;
	
	// Reset position to base position in new map instance
	float mapOffset = m_currentMapInstance * _mapPixelWidth;
	m_worldX = m_baseX + mapOffset;
	m_worldY = m_baseY;
	
	// Randomize coin texture on respawn
	if (m_animLoader)
	{
		delete m_animLoader;
		m_animLoader = new AnimatedSpriteLoader();
		CoinTextureInfo texInfo = GetRandomCoinTexture();
		m_animLoader->LoadAnimation("idle", texInfo.path, 1, texInfo.frameCount, 16, 16, texInfo.frameCount, 10.0f);
	}
	
	// Reactivate coin
	m_isActive = true;
}

vector<Coin*> Coin::SpawnCoinsFromMap(GameMap* _map)
{
	vector<Coin*> coins;
	
	if (!_map)
		return coins;
	
	const vector<pair<float, float>>& spawnPoints = _map->GetCoinSpawnPoints();
	
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
	
	// Convert world position to screen position
	float screenX = _camera ? _camera->WorldToScreenX(m_worldX) : m_worldX;
	float screenY = m_worldY;
	
	// Only render if on screen (with buffer for smooth appearance)
	Point screenSize = _renderer->GetWindowSize();
	if (screenX < -width || screenX > screenSize.X + width)
		return;
	
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
