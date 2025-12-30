#ifndef WORLD_ENTITY_H
#define WORLD_ENTITY_H

#include "../Core/StandardIncludes.h"
#include "../Graphics/Renderer.h"
#include "../Graphics/AnimatedSpriteLoader.h"

class Camera;

// Base class for game objects that exist in the world (enemies, coins, etc.)
class WorldEntity
{
public:
	WorldEntity();
	virtual ~WorldEntity();

	// Core methods - override in derived classes
	virtual void Initialize(float x, float y) = 0;
	virtual void Update(float _deltaTime, float _cameraX, int _screenWidth, int _mapPixelWidth);
	virtual void Render(Renderer* _renderer, Camera* _camera);
	virtual void Reset();
	
	// Position accessors
	float GetWorldX() const { return m_worldX; }
	float GetWorldY() const { return m_worldY; }
	float GetBaseX() const { return m_baseX; }
	float GetBaseY() const { return m_baseY; }
	virtual float GetWidth() const { return 16.0f; }
	virtual float GetHeight() const { return 16.0f; }
	
	// State
	bool IsActive() const { return m_isActive; }
	void SetActive(bool active) { m_isActive = active; }
	
	// Collision
	void GetCollisionBox(float& outX, float& outY, float& outWidth, float& outHeight) const;
	
	// Repositioning for infinite scrolling
	void RepositionAhead(float _cameraX, int _screenWidth, int _mapPixelWidth);

protected:
	// Called when entity is repositioned ahead - override to customize behavior
	virtual void OnRepositionAhead() {}
	
	// Get current animation name for rendering
	virtual string GetCurrentAnimationName() const { return "idle"; }
	
	// Set base position from spawn point
	void SetBasePosition(float x, float y);
	
	AnimatedSpriteLoader* m_animLoader;
	float m_worldX;
	float m_worldY;
	float m_baseX;
	float m_baseY;
	bool m_isActive;
	int m_currentMapInstance;
};

#endif // WORLD_ENTITY_H
