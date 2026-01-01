# Game Engine Architecture

## How It Works

```
┌─────────────────────────────────────────────────────────────┐
│                         ENTITY                              │
│             (just a container with unique ID)               │
│                                                             │
│   ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│   │  Transform   │  │    Sprite    │  │   Movement   │      │
│   │  Component   │  │  Component   │  │  Component   │      │
│   │              │  │              │  │              │      │
│   │  x = 100     │  │  anim: run   │  │  velX = 50   │      │
│   │  y = 200     │  │  flip: right │  │  velY = 0    │      │
│   └──────────────┘  └──────────────┘  └──────────────┘      │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                         SYSTEMS                             │
│             (read/write component data every frame)         │
│                                                             │
│   InputSystem ──► PhysicsSystem ──► MovementSystem ──►      │
│   CollisionSystem ──► AnimationSystem ──► RenderSystem      │
└─────────────────────────────────────────────────────────────┘
```

---

## Player vs Enemy - Same Components, Different Behavior

```
PLAYER                              ENEMY
┌────────────────────┐              ┌────────────────────┐
│ TransformComponent │              │ TransformComponent │
│ SpriteComponent    │              │ SpriteComponent    │
│ MovementComponent  │              │ MovementComponent  │
│ PhysicsComponent   │              │ CollisionComponent │
│ JumpComponent      │              │ PatrolComponent    │  ← Enemy patrols
│ CollisionComponent │              │ EnemyComponent     │
│ HealthComponent    │              │ ScrollableComponent│
│ InputComponent     │  ← Player    └────────────────────┘
│ PlayerTag          │    has input
└────────────────────┘

InputSystem  → only processes entities with InputComponent (Player)
PatrolSystem → only processes entities with PatrolComponent (Enemy)
```

---

## File Structure

```
Game/
├── Entity.h/cpp         - Entity class (ID + components)
├── Components.h         - All components (data)
├── Systems.h/cpp        - All systems (logic)
├── EntityManager.h/cpp  - Creates and manages entities
├── ChunkMap.h/cpp       - Infinite scrolling map
├── SpatialGrid.h/cpp    - Spatial partitioning for collision
├── Level.h/cpp          - Serializable level data
├── Unit.h/cpp           - Serializable unit with object pooling
└── GameUI.h/cpp         - UI rendering

Core/
└── GameController.h/cpp - Main game loop

Graphics/                - Renderer, Camera, Sprites
Audio/                   - Sound, Music
Input/                   - Keyboard, Mouse
```

---

## Components

| Component | Purpose |
|-----------|---------|
| `TransformComponent` | Position, size, scale |
| `SpriteComponent` | Animation and rendering |
| `MovementComponent` | Velocity and speed |
| `PhysicsComponent` | Gravity and ground state |
| `JumpComponent` | Jump mechanics |
| `CollisionComponent` | Collision box |
| `HealthComponent` | Health and death state |
| `PatrolComponent` | AI patrol boundaries |
| `CollectibleComponent` | Coin/item data |
| `EnemyComponent` | Enemy type |
| `PlayerTag` | Identifies player entity |
| `ScrollableComponent` | Infinite scroll repositioning |

---

## Systems

| System | Purpose |
|--------|---------|
| `InputSystem` | Keyboard → player velocity |
| `PhysicsSystem` | Apply gravity |
| `JumpSystem` | Handle jump input |
| `DashSystem` | Handle dash ability |
| `PunchSystem` | Handle punch attack |
| `MovementSystem` | Velocity → position |
| `CollisionSystem` | Player vs world tiles |
| `PatrolSystem` | Enemy patrol movement |
| `ScrollSystem` | Infinite scroll repositioning |
| `HealthSystem` | Invincibility and death timers |
| `EntityCollisionSystem` | Player vs enemies/coins (uses SpatialGrid) |
| `AnimationSystem` | Update sprite animation |
| `RenderSystem` | Draw sprites |

---

## Spatial Partitioning (SpatialGrid)

```
┌─────────────────────────────────────────────────────────────┐
│                    WORLD SPACE                              │
│  ┌────────┬────────┬────────┬────────┐                      │
│  │ Cell   │ Cell   │ Cell   │ Cell   │                      │
│  │ (0,0)  │ (1,0)  │ (2,0)  │ (3,0)  │                      │
│  │        │ [P]    │        │        │  P = Player          │
│  ├────────┼────────┼────────┼────────┤  E = Enemy           │
│  │ Cell   │ Cell   │ Cell   │ Cell   │  C = Coin            │
│  │ (0,1)  │ (1,1)  │ (2,1)  │ (3,1)  │                      │
│  │        │ [E]    │ [C]    │        │                      │
│  └────────┴────────┴────────┴────────┘                      │
└─────────────────────────────────────────────────────────────┘

Collision check: Player only checks cells (1,0), (2,0), (1,1), (2,1)
Instead of checking ALL entities in the world → O(n) vs O(n²)
```

Two-phase collision detection:
1. **Broad-phase**: AABB overlap test (fast rejection)
2. **Narrow-phase**: Detailed collision (only if broad-phase passes)

### How a System Works

```cpp
// Each system loops through all entities
// and processes only those with required components

void MovementSystem::Update(entities, deltaTime)
{
    for (entity : entities)
    {
        // Get required components
        auto* transform = entity->GetComponent<TransformComponent>();
        auto* movement = entity->GetComponent<MovementComponent>();
        
        // Skip if entity doesn't have these components
        if (!transform || !movement) continue;
        
        // Update position based on velocity
        transform->worldX += movement->velocityX * deltaTime;
        transform->worldY += movement->velocityY * deltaTime;
    }
}
```

---

## Creating Entities

```cpp
Entity* player = EntityFactory::CreatePlayer(x, y);
Entity* enemy = EntityFactory::CreateEnemy(x, y, EnemyVariant::Ghost, leftBound, rightBound);
Entity* coin = EntityFactory::CreateCoin(x, y, CollectibleType::Coin1);
```

---

## Game Loop

```cpp
while (running)
{
    entityManager.Update(deltaTime);  // Runs all systems
    chunkMap->Update(cameraX, screenWidth);
    
    chunkMap->RenderBackgrounds(renderer, camera);
    chunkMap->Render(renderer, camera);
    entityManager.Render(renderer, camera);
    gameUI->Render(renderer, score, health, maxHealth);
}
```

---

## Example: Adding New Features

### 1. Add Component
```cpp
// Components.h
struct DashComponent : Component
{
    float dashSpeed = 500;
    bool isDashing = false;
};
```

### 2. Add System
```cpp
// Systems.h
class DashSystem : public System
{
public:
    void Update(std::vector<Entity*>& entities, float deltaTime) override;
};
```

### 3. Register in EntityManager
```cpp
// EntityManager.h
DashSystem m_dash;

// EntityManager::Update()
m_dash.Update(m_entities, deltaTime);
```

### 4. Add to Entity
```cpp
// EntityFactory::CreatePlayer()
entity->AddComponent<DashComponent>();
```
