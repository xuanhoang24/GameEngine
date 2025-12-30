#ifndef ENEMY_H
#define ENEMY_H

#include "../Game/WorldEntity.h"

enum class EnemyType
{
	Ghost,
	Mushroom
};

class Enemy : public WorldEntity
{
public:
	Enemy();
	virtual ~Enemy();

	void Initialize(float x, float y) override;
	void Initialize(float x, float y, EnemyType type, float leftBoundary, float rightBoundary);
	void Update(float _deltaTime, float _cameraX, int _screenWidth, int _mapPixelWidth) override;
	void Reset() override;
	
	void Destroy() { m_isActive = false; }
	EnemyType GetType() const { return m_type; }

protected:
	void OnRepositionAhead() override;

private:
	void RandomEnemy();
	
	EnemyType m_type;
	
	// Movement
	float m_moveSpeed;
	float m_direction;
	
	// Zone boundaries (base values within single map)
	float m_baseLeftBoundary;
	float m_baseRightBoundary;
};

#endif // ENEMY_H
