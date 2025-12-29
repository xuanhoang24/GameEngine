#ifndef PLAYER_H
#define PLAYER_H

#include "../Core/StandardIncludes.h"
#include "../Graphics/Renderer.h"
#include "../Graphics/AnimatedSpriteLoader.h"

class ChunkMap;
class Camera;

class Player
{
public:
	Player();
	virtual ~Player();

	void Initialize();
	void Update(float _deltaTime);
	void Render(Renderer* _renderer, Camera* _camera);
	void RenderCollisionBox(Renderer* _renderer, Camera* _camera);
	void HandleInput(SDL_Event _event);
	void SetChunkMap(ChunkMap* _map) { m_chunkMap = _map; }
	void SetSpawnPosition(float x, float y);
	
	float GetWidth() const { return 16 * scale; }
	float GetHeight() const { return 16 * scale; }
	float GetWorldX() const { return m_worldX; }
	float GetWorldY() const { return m_position.Y; }

	void GetCollisionBox(float& outX, float& outY, float& outWidth, float& outHeight) const;
	
	bool IsMovingDown() const { return m_veloY > 0; }
	void Die();
	bool IsDead() const { return m_isDead; }
	bool IsFullyDead() const { return m_isFullyDead; }
	void Reset();

private:
	bool m_isDead;
	bool m_isFullyDead;
	float m_deathTimer;
	float m_deathDuration;
	AnimatedSpriteLoader* m_animLoader;
	Point m_position;
	float m_worldX;
	float scale;
	
	// Movement
	float m_walkSpeed;
	float m_runSpeed;
	float m_veloX;
	float m_veloY;
	bool m_isRunning;
	bool m_shiftDown;
	bool m_facingRight;

	// Physics
	float m_gravity;
	bool m_isGrounded;
	ChunkMap* m_chunkMap;

	// Jump
	bool m_jumpPressed;
	bool m_isJumping;
	float m_jumpForce;
	float m_jumpHoldForce;
	float m_jumpMaxHoldTime;
	float m_jumpHoldTimer;
	float m_coyoteTime;
	float m_coyoteTimer;
};

#endif // PLAYER_H

