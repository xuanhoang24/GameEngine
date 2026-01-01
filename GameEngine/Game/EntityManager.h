#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

#include "Entity.h"
#include "Components.h"
#include "Systems.h"
#include <vector>

class Renderer;
class Camera;
class ChunkMap;

/**
 * Factory for creating pre-configured entities.
 * Each method creates an entity with the appropriate components.
 */
class EntityFactory
{
public:
    static Entity* CreatePlayer(float x, float y);
    static Entity* CreateEnemy(float x, float y, EnemyVariant type, float left, float right);
    static Entity* CreateCoin(float x, float y, CollectibleType type);
    static Entity* CreateRandomCoin(float x, float y);
    static Entity* CreateRandomEnemy(float x, float y, float left, float right);
};

/**
 * Manages all entities and systems.
 * 
 * Responsibilities:
 * - Create and destroy entities
 * - Run all systems each frame in correct order
 * - Provide access to player entity
 */
class EntityManager
{
public:
    EntityManager() = default;
    ~EntityManager();

    Entity* CreateEntity();
    void DestroyEntity(Entity* entity);
    Entity* GetPlayer();
    std::vector<Entity*>& GetAllEntities() { return m_entities; }

    void SetChunkMap(ChunkMap* map);
    void SetScrollParams(float cameraX, int screenWidth, int mapWidth);

    // Run all systems in order
    void Update(float deltaTime);
    void Render(Renderer* renderer, Camera* camera);

    int GetScore() const { return m_entityCollision.GetScore(); }
    void ResetScore() { m_entityCollision.ResetScore(); }
    
    // Debug visualization for spatial grid
    void ToggleSpatialGridDebug() { m_entityCollision.ToggleDebugDraw(); }
    bool IsSpatialGridDebugEnabled() const { return m_entityCollision.IsDebugDrawEnabled(); }
    void RenderSpatialGridDebug(Renderer* renderer, Camera* camera, float viewportWidth, float viewportHeight);
    
    // Debug visualization for collision boxes (F2)
    void ToggleCollisionBoxDebug() { m_collisionBoxDebugEnabled = !m_collisionBoxDebugEnabled; }
    bool IsCollisionBoxDebugEnabled() const { return m_collisionBoxDebugEnabled; }
    void RenderCollisionBoxDebug(Renderer* renderer, Camera* camera);

    void Reset();
    void Clear();

private:
    std::vector<Entity*> m_entities;
    std::vector<Entity*> m_pendingDestroy;

    // Systems are executed in this order each frame
    InputSystem m_input;
    PhysicsSystem m_physics;
    JumpSystem m_jump;
    DashSystem m_dash;
    PunchSystem m_punch;
    MovementSystem m_movement;
    CollisionSystem m_collision;
    PatrolSystem m_patrol;
    ScrollSystem m_scroll;
    HealthSystem m_health;
    EntityCollisionSystem m_entityCollision;
    AnimationSystem m_animation;
    RenderSystem m_render;

    void ProcessDestroys();
    
    bool m_collisionBoxDebugEnabled = false;
};

#endif
