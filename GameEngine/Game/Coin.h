#ifndef COIN_H
#define COIN_H

#include "../Core/StandardIncludes.h"
#include "../Graphics/Renderer.h"
#include "../Graphics/AnimatedSpriteLoader.h"

class Camera;
class GameMap;

enum class CoinType
{
	Coin1,    // 10 points
	Coin2,    // 5 points
	Diamond   // 15 points
};

struct CoinTextureInfo
{
	string path;
	int frameCount;
	CoinType type;
	int pointValue;
};

class Coin
{
public:
	Coin();
	virtual ~Coin();

	void Initialize(float x, float y);
	void Update(float _deltaTime, float _cameraX, int _screenWidth, int _mapPixelWidth);
	void Render(Renderer* _renderer, Camera* _camera);
	
	static vector<Coin*> SpawnCoinsFromMap(GameMap* _map);
	
	float GetWorldX() const { return m_worldX; }
	float GetWorldY() const { return m_worldY; }
	float GetBaseX() const { return m_baseX; } // Original spawn X within map
	float GetBaseY() const { return m_baseY; } // Original spawn Y
	float GetWidth() const { return 16.0f; }
	float GetHeight() const { return 16.0f; }
	
	bool IsActive() const { return m_isActive; }
	void Collect() { m_isActive = false; }
	
	CoinType GetCoinType() const { return m_coinType; }
	int GetPointValue() const { return m_pointValue; }
	
	// Reposition coin to next map instance in front of camera
	void RepositionAhead(float _cameraX, int _screenWidth, int _mapPixelWidth);
	
	void GetCollisionBox(float& _outX, float& _outY, float& _outWidth, float& _outHeight) const
	{
		_outX = m_worldX;
		_outY = m_worldY;
		_outWidth = GetWidth();
		_outHeight = GetHeight();
	}

private:
	void SetCoinTexture(const CoinTextureInfo& texInfo);
	
	AnimatedSpriteLoader* m_animLoader;
	float m_worldX;      // Current world X position
	float m_worldY;      // Current world Y position
	float m_baseX;       // Original spawn X within single map (0 to mapWidth)
	float m_baseY;       // Original spawn Y
	bool m_isActive;
	int m_currentMapInstance; // Which map instance this coin is currently in
	CoinType m_coinType;
	int m_pointValue;
};

#endif // COIN_H
