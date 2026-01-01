#include "EntityManager.h"
#include "ChunkMap.h"
#include <algorithm>
#include <random>

Entity* EntityFactory::CreatePlayer(float x, float y)
{
    Entity* entity = new Entity();

    auto* transform = entity->AddComponent<TransformComponent>();
    transform->worldX = transform->baseX = x;
    transform->worldY = transform->baseY = y;

    auto* sprite = entity->AddComponent<SpriteComponent>();
    sprite->animLoader = new AnimatedSpriteLoader();
    sprite->animLoader->LoadAnimation("idle", "../Assets/Textures/Player/idle.png", 1, 4, 16, 16, 4, 8.0f);
    sprite->animLoader->LoadAnimation("run", "../Assets/Textures/Player/run.png", 1, 4, 16, 16, 4, 12.0f);
    sprite->animLoader->LoadAnimation("jumpandfall", "../Assets/Textures/Player/jumpandfall.png", 1, 2, 16, 16, 2, 8.0f);
    sprite->animLoader->LoadAnimation("hurt", "../Assets/Textures/Player/hurt.png", 1, 2, 16, 16, 2, 2.0f);
    sprite->animLoader->LoadAnimation("punch", "../Assets/Textures/Player/punch1.png", 1, 4, 16, 16, 4, 12.0f);

    entity->AddComponent<MovementComponent>();
    entity->AddComponent<PhysicsComponent>();
    entity->AddComponent<JumpComponent>();
    entity->AddComponent<DashComponent>();
    entity->AddComponent<PunchComponent>();
    entity->AddComponent<CollisionComponent>()->type = ColliderType::Player;
    entity->AddComponent<HealthComponent>();
    entity->AddComponent<InputComponent>();
    entity->AddComponent<PlayerTag>();

    return entity;
}

Entity* EntityFactory::CreateEnemy(float x, float y, EnemyVariant type, float left, float right)
{
    Entity* entity = new Entity();

    auto* transform = entity->AddComponent<TransformComponent>();
    transform->baseX = transform->worldX = x - 8;
    transform->baseY = transform->worldY = y - 16;

    auto* sprite = entity->AddComponent<SpriteComponent>();
    sprite->animLoader = new AnimatedSpriteLoader();

    auto* movement = entity->AddComponent<MovementComponent>();
    movement->direction = (rand() % 2) ? -1.0f : 1.0f;

    auto* enemy = entity->AddComponent<EnemyComponent>();
    enemy->variant = type;

    if (type == EnemyVariant::Ghost)
    {
        sprite->animLoader->LoadAnimation("idle", "../Assets/Textures/Enemy/ghost1_fly.png", 1, 6, 16, 16, 6, 10.0f);
        movement->moveSpeed = 30;
    }
    else
    {
        sprite->animLoader->LoadAnimation("idle", "../Assets/Textures/Enemy/mushroom-walk.png", 1, 10, 16, 16, 10, 10.0f);
        movement->moveSpeed = 40;
    }

    auto* patrol = entity->AddComponent<PatrolComponent>();
    patrol->baseLeftBoundary = left;
    patrol->baseRightBoundary = right;

    auto* collision = entity->AddComponent<CollisionComponent>();
    collision->type = ColliderType::Enemy;
    collision->isTrigger = true;

    auto* scrollable = entity->AddComponent<ScrollableComponent>();
    scrollable->onReposition = [](Entity* ent) {
        auto* mov = ent->GetComponent<MovementComponent>();
        if (mov) mov->direction = (rand() % 2) ? -1.0f : 1.0f;
    };

    return entity;
}

Entity* EntityFactory::CreateCoin(float x, float y, CollectibleType type)
{
    Entity* entity = new Entity();

    auto* transform = entity->AddComponent<TransformComponent>();
    transform->baseX = transform->worldX = x - 8;
    transform->baseY = transform->worldY = y - 16;

    auto* sprite = entity->AddComponent<SpriteComponent>();
    sprite->animLoader = new AnimatedSpriteLoader();

    auto* collectible = entity->AddComponent<CollectibleComponent>();
    collectible->type = type;

    if (type == CollectibleType::Coin1)
    {
        sprite->animLoader->LoadAnimation("idle", "../Assets/Textures/Obstacles/coin1.png", 1, 10, 16, 16, 10, 10.0f);
        collectible->pointValue = 10;
    }
    else if (type == CollectibleType::Coin2)
    {
        sprite->animLoader->LoadAnimation("idle", "../Assets/Textures/Obstacles/coin2.png", 1, 10, 16, 16, 10, 10.0f);
        collectible->pointValue = 5;
    }
    else
    {
        sprite->animLoader->LoadAnimation("idle", "../Assets/Textures/Obstacles/diamond.png", 1, 5, 16, 16, 5, 10.0f);
        collectible->pointValue = 15;
    }

    auto* collision = entity->AddComponent<CollisionComponent>();
    collision->type = ColliderType::Coin;
    collision->isTrigger = true;

    entity->AddComponent<ScrollableComponent>();

    return entity;
}

Entity* EntityFactory::CreateRandomCoin(float x, float y)
{
    static std::mt19937 gen(std::random_device{}());
    static std::uniform_int_distribution<> dist(0, 2);
    CollectibleType types[] = { CollectibleType::Coin1, CollectibleType::Coin2, CollectibleType::Diamond };
    return CreateCoin(x, y, types[dist(gen)]);
}

Entity* EntityFactory::CreateRandomEnemy(float x, float y, float left, float right)
{
    return CreateEnemy(x, y, (rand() % 2) ? EnemyVariant::Ghost : EnemyVariant::Mushroom, left, right);
}

EntityManager::~EntityManager() { Clear(); }

Entity* EntityManager::CreateEntity()
{
    Entity* entity = new Entity();
    m_entities.push_back(entity);
    return entity;
}

void EntityManager::DestroyEntity(Entity* entity)
{
    if (entity) m_pendingDestroy.push_back(entity);
}

Entity* EntityManager::GetPlayer()
{
    for (auto* entity : m_entities)
        if (entity && entity->HasComponent<PlayerTag>()) return entity;
    return nullptr;
}

void EntityManager::SetChunkMap(ChunkMap* map)
{
    m_collision.SetChunkMap(map);
    if (map) m_patrol.SetMapWidth(map->GetChunkPixelWidth());
}

void EntityManager::SetScrollParams(float camX, int screenW, int mapW)
{
    m_scroll.SetParams(camX, screenW, mapW);
    m_patrol.SetMapWidth(mapW);
}

void EntityManager::Update(float deltaTime)
{
    if (deltaTime > 0.033f) deltaTime = 0.033f;

    m_input.Update(m_entities, deltaTime);
    m_physics.Update(m_entities, deltaTime);
    m_jump.Update(m_entities, deltaTime);
    m_dash.Update(m_entities, deltaTime);
    m_punch.Update(m_entities, deltaTime);
    m_movement.Update(m_entities, deltaTime);
    m_collision.Update(m_entities, deltaTime);
    m_patrol.Update(m_entities, deltaTime);
    m_scroll.Update(m_entities, deltaTime);
    m_health.Update(m_entities, deltaTime);
    
    // Rebuild spatial grid after all movement is done
    m_entityCollision.RebuildGrid(m_entities);
    m_entityCollision.Update(m_entities, deltaTime);
    m_animation.Update(m_entities, deltaTime);

    // Fall death
    if (Entity* player = GetPlayer())
    {
        auto* transform = player->GetComponent<TransformComponent>();
        auto* health = player->GetComponent<HealthComponent>();
        if (transform && health && transform->worldY > 260 && !health->isDead)
        {
            health->isDead = true;
            health->health = 0;
            health->deathTimer = 0;
        }
    }

    ProcessDestroys();
}

void EntityManager::Render(Renderer* renderer, Camera* camera)
{
    m_render.Render(m_entities, renderer, camera);
}

void EntityManager::RenderSpatialGridDebug(Renderer* renderer, Camera* camera, float viewportWidth, float viewportHeight)
{
    m_entityCollision.RenderDebug(renderer, camera, viewportWidth, viewportHeight);
}

void EntityManager::RenderCollisionBoxDebug(Renderer* renderer, Camera* camera)
{
    if (!m_collisionBoxDebugEnabled) return;
    
    for (auto* entity : m_entities)
    {
        if (!entity || !entity->IsActive()) continue;
        
        auto* transform = entity->GetComponent<TransformComponent>();
        auto* collision = entity->GetComponent<CollisionComponent>();
        if (!transform || !collision) continue;
        
        float worldX = transform->worldX + collision->offsetX;
        float worldY = transform->worldY + collision->offsetY;
        float screenX = camera ? camera->WorldToScreenX(worldX) : worldX;
        float screenY = worldY;
        
        // Different colors for different collider types
        if (entity->HasComponent<PlayerTag>())
            renderer->SetDrawColor(Color(0, 255, 0, 255));  // Green for player
        else if (collision->type == ColliderType::Enemy)
            renderer->SetDrawColor(Color(255, 0, 0, 255));  // Red for enemies
        else if (collision->type == ColliderType::Coin)
            renderer->SetDrawColor(Color(255, 255, 0, 255));  // Yellow for coins
        else
            renderer->SetDrawColor(Color(255, 255, 255, 255));  // White for others
        
        Rect collisionRect(
            static_cast<unsigned int>(screenX),
            static_cast<unsigned int>(screenY),
            static_cast<unsigned int>(screenX + collision->boxWidth),
            static_cast<unsigned int>(screenY + collision->boxHeight)
        );
        renderer->RenderRectangle(collisionRect);
    }
}

void EntityManager::ProcessDestroys()
{
    for (auto* entity : m_pendingDestroy)
    {
        auto it = std::find(m_entities.begin(), m_entities.end(), entity);
        if (it != m_entities.end())
        {
            delete *it;
            m_entities.erase(it);
        }
    }
    m_pendingDestroy.clear();
}

void EntityManager::Reset()
{
    for (auto* entity : m_entities)
    {
        if (!entity) continue;
        auto* transform = entity->GetComponent<TransformComponent>();
        auto* movement = entity->GetComponent<MovementComponent>();
        auto* physics = entity->GetComponent<PhysicsComponent>();
        auto* jump = entity->GetComponent<JumpComponent>();
        auto* health = entity->GetComponent<HealthComponent>();
        auto* sprite = entity->GetComponent<SpriteComponent>();

        if (transform)
        {
            transform->worldX = transform->baseX;
            transform->worldY = transform->baseY;
            transform->mapInstance = 0;
        }
        if (movement)
        {
            movement->velocityX = 0;
            movement->velocityY = 0;
        }
        if (physics) physics->isGrounded = false;
        if (jump)
        {
            jump->isJumping = false;
            jump->jumpPressed = false;
            jump->jumpHoldTimer = 0;
            jump->coyoteTimer = 0;
        }
        auto* dash = entity->GetComponent<DashComponent>();
        if (dash)
        {
            dash->isDashing = false;
            dash->dashPressed = false;
            dash->dashTimer = 0;
            dash->cooldownTimer = 0;
        }
        auto* punch = entity->GetComponent<PunchComponent>();
        if (punch)
        {
            punch->isPunching = false;
            punch->punchPressed = false;
            punch->punchTimer = 0;
            punch->hasHit = false;
        }
        if (health)
        {
            health->health = health->maxHealth;
            health->isDead = false;
            health->isFullyDead = false;
            health->isInvincible = false;
            health->deathTimer = 0;
            health->invincibleTimer = 0;
        }
        if (sprite)
        {
            sprite->facingRight = true;
            sprite->flickering = false;
            sprite->flickerCounter = 0;
        }
        entity->SetActive(true);
    }
}

void EntityManager::Clear()
{
    for (auto* entity : m_entities) delete entity;
    m_entities.clear();
    m_pendingDestroy.clear();
}
