#ifndef ENEMY_H
#define ENEMY_H

#include "../Core/StandardIncludes.h"
#include "../Graphics/Renderer.h"
#include "../Graphics/AnimatedSpriteLoader.h"

class Camera;
class GameMap;

enum class EnemyType
{
	Ghost,
	Mushroom
};

class Enemy
{
public:
	Enemy();
	virtual ~Enemy();

	void Initialize(float x, float y, EnemyType type);
	void Initialize(float x, float y, EnemyType type, float leftBoundary, float rightBoundary);
	void Update(float _deltaTime);
	void Render(Renderer* _renderer, Camera* _camera);
	
	static vector<Enemy*> SpawnEnemiesFromMap(GameMap* _map);
	
	void SetGameMap(GameMap* _map) { m_gameMap = _map; }
	
	float GetWorldX() const { return m_worldX; }
	float GetWorldY() const { return m_worldY; }
	float GetWidth() const { return 16.0f; }
	float GetHeight() const { return 16.0f; }
	
	bool IsActive() const { return m_isActive; }
	void Destroy() { m_isActive = false; }
	void CheckRespawn(float cameraX, int mapPixelWidth);
	
	EnemyType GetType() const { return m_type; }
	
	void GetCollisionBox(float& outX, float& outY, float& outWidth, float& outHeight) const
	{
		outX = m_worldX;
		outY = m_worldY;
		outWidth = GetWidth();
		outHeight = GetHeight();
	}

private:
	AnimatedSpriteLoader* m_animLoader;
	float m_worldX;
	float m_worldY;
	bool m_isActive;
	int m_destroyedInMapLoop; // Track which map loop enemy was destroyed in (-1 = never destroyed)
	EnemyType m_type;
	float m_lastCameraX;
	
	// Movement
	float m_moveSpeed;
	float m_direction; // -1 for left, 1 for right
	
	// Zone boundaries (from tilemap)
	float m_leftBoundary;
	float m_rightBoundary;
	
	// Map collision (optional)
	GameMap* m_gameMap;
};

#endif // ENEMY_H
