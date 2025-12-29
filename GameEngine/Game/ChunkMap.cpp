#include "../Game/ChunkMap.h"
#include "../Graphics/Renderer.h"
#include "../Game/Coin.h"
#include "../Game/Enemy.h"
#include "../Core/Timing.h"

ChunkMap::ChunkMap()
    : m_startChunk(nullptr)
    , m_nextChunkX(0.0f)
    , m_chunkWidth(0)
    , m_rng(std::random_device{}())
    , m_dist(1, 100)
    , m_floatDist(0.0f, 1.0f)
{
}

ChunkMap::~ChunkMap()
{
    // Cleanup all entities in active chunks
    for (auto& chunk : m_activeChunks)
    {
        CleanupChunkEntities(chunk);
    }
    
    delete m_startChunk;
    
    for (auto* chunk : m_randomChunks)
        delete chunk;
    m_randomChunks.clear();
    
    for (auto* chunk : m_gapChunks)
        delete chunk;
    m_gapChunks.clear();
    
    for (auto* chunk : m_floatingChunks)
        delete chunk;
    m_floatingChunks.clear();
    
    m_activeChunks.clear();
}

bool ChunkMap::Load(const string& _startChunkPath)
{
    m_startChunk = new TileMap();
    if (!m_startChunk->Load(_startChunkPath))
        return false;
    
    // Get chunk width from start chunk
    m_chunkWidth = m_startChunk->GetMapPixelWidth();
    
    // Spawn the start chunk at position 0
    ChunkInstance startInstance;
    startInstance.tileMap = m_startChunk;
    startInstance.worldOffsetX = 0.0f;
    startInstance.chunkType = 0;
    m_activeChunks.push_back(startInstance);
    
    m_nextChunkX = (float)m_chunkWidth;
    
    return true;
}

void ChunkMap::AddRandomChunk(const string& _path)
{
    TileMap* chunk = new TileMap();
    if (chunk->Load(_path))
        m_randomChunks.push_back(chunk);
    else
        delete chunk;
}

void ChunkMap::AddGapChunk(const string& _path)
{
    TileMap* chunk = new TileMap();
    if (chunk->Load(_path))
        m_gapChunks.push_back(chunk);
    else
        delete chunk;
}

void ChunkMap::AddFloatingChunk(const string& _path)
{
    TileMap* chunk = new TileMap();
    if (chunk->Load(_path))
        m_floatingChunks.push_back(chunk);
    else
        delete chunk;
}

void ChunkMap::LoadDefaultChunks()
{
    // Load start chunk
    Load("../Assets/Maps/Chunk/chunk_flat_start.tmx");
    
    // Add random chunks
    AddRandomChunk("../Assets/Maps/Chunk/chunk_random_01.tmx");
    AddRandomChunk("../Assets/Maps/Chunk/chunk_random_11.tmx");
    AddRandomChunk("../Assets/Maps/Chunk/chunk_random_21.tmx");
    
    // Add gap chunks
    AddGapChunk("../Assets/Maps/Chunk/chunk_gap_01.tmx");
    
    // Add floating chunks
    AddFloatingChunk("../Assets/Maps/Chunk/chunk_floating_01.tmx");
    AddFloatingChunk("../Assets/Maps/Chunk/chunk_floating_02.tmx");
    AddFloatingChunk("../Assets/Maps/Chunk/chunk_floating_11.tmx");
    AddFloatingChunk("../Assets/Maps/Chunk/chunk_floating_12.tmx");
}

void ChunkMap::Update(float _cameraX, float _screenWidth)
{
    // Spawn new chunks ahead of camera
    float spawnThreshold = _cameraX + _screenWidth + m_chunkWidth;
    
    while (m_nextChunkX < spawnThreshold)
    {
        SpawnNextChunk();
    }
    
    // Remove chunks that are far behind camera (cleanup)
    float despawnThreshold = _cameraX - m_chunkWidth * 2;
    
    auto it = m_activeChunks.begin();
    while (it != m_activeChunks.end())
    {
        // Not remove template chunks, just remove from active list
        if (it->worldOffsetX + m_chunkWidth < despawnThreshold && it->chunkType != 0)
        {
            CleanupChunkEntities(*it);
            it = m_activeChunks.erase(it);
        }
        else
        {
            ++it;
        }
    }
    
    // Update all coins and enemies in active chunks
    for (auto& chunk : m_activeChunks)
    {
        for (auto* coin : chunk.coins)
        {
            if (coin && coin->IsActive())
            {
                coin->Update(Timing::Instance().GetDeltaTime(), _cameraX, (int)_screenWidth, m_chunkWidth);
            }
        }
        for (auto* enemy : chunk.enemies)
        {
            if (enemy && enemy->IsActive())
            {
                enemy->Update(Timing::Instance().GetDeltaTime(), _cameraX, (int)_screenWidth, m_chunkWidth);
            }
        }
    }
}

void ChunkMap::SpawnNextChunk()
{
    ChunkInstance newChunk;
    newChunk.worldOffsetX = m_nextChunkX;
    newChunk.chunkType = SelectRandomChunkType();
    newChunk.tileMap = SelectRandomChunkVariant(newChunk.chunkType);
    
    if (newChunk.tileMap)
    {
        // Spawn entities based on spawn zones
        SpawnEntitiesForChunk(newChunk);
        m_activeChunks.push_back(newChunk);
    }
    
    m_nextChunkX += m_chunkWidth;
}

int ChunkMap::SelectRandomChunkType()
{
    int roll = m_dist(m_rng);
    // 30% random, 30% gap, 30% floating, 10% fallback to random
    if (roll <= 30)
        return 1; // random
    else if (roll <= 60)
        return 2; // gap
    else if (roll <= 90)
        return 3; // floating
    else
        return 1; // fallback to random
}

TileMap* ChunkMap::SelectRandomChunkVariant(int _type)
{
    vector<TileMap*>* chunks = nullptr;
    
    switch (_type)
    {
        case 1: chunks = &m_randomChunks; break;
        case 2: chunks = &m_gapChunks; break;
        case 3: chunks = &m_floatingChunks; break;
        default: return nullptr;
    }
    
    if (chunks->empty())
    {
        // Fallback to random chunks if requested type is empty
        if (!m_randomChunks.empty())
            chunks = &m_randomChunks;
        else
            return nullptr;
    }
    
    std::uniform_int_distribution<size_t> dist(0, chunks->size() - 1);
    return (*chunks)[dist(m_rng)];
}

void ChunkMap::SpawnEntitiesForChunk(ChunkInstance& _chunk)
{
    if (!_chunk.tileMap) return;
    
    // Spawn coins from coin spawn zones (all chunk types)
    const auto& coinZones = _chunk.tileMap->GetCoinSpawnZones();
    for (const auto& zone : coinZones)
    {
        // Check spawn chance
        float roll = m_floatDist(m_rng);
        if (roll > zone.chance)
            continue;
        
        // Determine count to spawn
        int count = zone.minCount;
        if (zone.maxCount > zone.minCount)
        {
            std::uniform_int_distribution<int> countDist(zone.minCount, zone.maxCount);
            count = countDist(m_rng);
        }
        
        // Spawn coins within the zone
        for (int i = 0; i < count; ++i)
        {
            // Random position within zone (center X, bottom Y for Initialize)
            float coinWidth = 16.0f;
            float coinHeight = 16.0f;
            std::uniform_real_distribution<float> xDist(zone.x + coinWidth * 0.5f, zone.x + zone.width - coinWidth * 0.5f);
            std::uniform_real_distribution<float> yDist(zone.y + coinHeight, zone.y + zone.height);
            
            float localX = xDist(m_rng);
            float localY = yDist(m_rng);
            
            // Convert to world position (Initialize expects center-bottom position)
            float worldX = localX + _chunk.worldOffsetX;
            float worldY = localY;
            
            Coin* coin = new Coin();
            coin->Initialize(worldX, worldY);
            _chunk.coins.push_back(coin);
        }
    }
    
    // Spawn enemies from enemy spawn zones
    const auto& enemyZones = _chunk.tileMap->GetEnemySpawnZones();
    for (const auto& zone : enemyZones)
    {
        // Check spawn chance
        float roll = m_floatDist(m_rng);
        if (roll > zone.chance)
            continue;
        
        // Spawn enemies within the zone
        for (int i = 0; i < zone.maxCount; ++i)
        {
            // Random position within zone (center X for Initialize)
            float enemyWidth = 16.0f;
            std::uniform_real_distribution<float> xDist(zone.x + enemyWidth * 0.5f, zone.x + zone.width - enemyWidth * 0.5f);
            
            float localX = xDist(m_rng);
            float localY = zone.y + zone.height; // Spawn at bottom of zone
            
            // Convert to world position
            float worldX = localX + _chunk.worldOffsetX;
            float worldY = localY;
            
            // Select enemy type based on weights
            EnemyType enemyType = EnemyType::Ghost;
            if (!zone.enemyTypes.empty() && !zone.enemyWeights.empty())
            {
                float weightRoll = m_floatDist(m_rng);
                float cumulative = 0.0f;
                for (size_t j = 0; j < zone.enemyTypes.size(); ++j)
                {
                    cumulative += (j < zone.enemyWeights.size()) ? zone.enemyWeights[j] : 0.5f;
                    if (weightRoll <= cumulative)
                    {
                        if (zone.enemyTypes[j] == "ghost")
                            enemyType = EnemyType::Ghost;
                        else if (zone.enemyTypes[j] == "mushroom")
                            enemyType = EnemyType::Mushroom;
                        break;
                    }
                }
            }
            
            // Calculate movement boundaries (zone bounds in world space)
            float leftBound = zone.x + _chunk.worldOffsetX;
            float rightBound = zone.x + zone.width + _chunk.worldOffsetX - enemyWidth;
            
            Enemy* enemy = new Enemy();
            enemy->Initialize(worldX, worldY, enemyType, leftBound, rightBound);
            _chunk.enemies.push_back(enemy);
        }
    }
}

void ChunkMap::CleanupChunkEntities(ChunkInstance& _chunk)
{
    for (auto* coin : _chunk.coins)
    {
        delete coin;
    }
    _chunk.coins.clear();
    
    for (auto* enemy : _chunk.enemies)
    {
        delete enemy;
    }
    _chunk.enemies.clear();
}

void ChunkMap::Render(Renderer* _renderer, Camera* _camera)
{
    float cameraX = _camera ? _camera->GetX() : 0.0f;
    
    Point logicalSize = _renderer->GetLogicalSize();
    int screenWidth = logicalSize.X;
    
    for (const auto& chunk : m_activeChunks)
    {
        // Check if chunk is visible
        float chunkRight = chunk.worldOffsetX + m_chunkWidth;
        if (chunkRight < cameraX || chunk.worldOffsetX > cameraX + screenWidth)
            continue;
        
        // Render this chunk with offset
        RenderChunkWithOffset(_renderer, _camera, chunk);
    }
}

void ChunkMap::RenderChunkWithOffset(Renderer* _renderer, Camera* _camera, const ChunkInstance& _chunk)
{
    if (!_chunk.tileMap) return;
    
    SDL_Renderer* sdl = _renderer->GetRenderer();
    float cameraX = _camera ? _camera->GetX() : 0.0f;
    
    TileMap* tileMap = _chunk.tileMap;
    float offsetX = _chunk.worldOffsetX;
    
    int mapWidth = tileMap->GetMapWidth();
    int mapHeight = tileMap->GetMapHeight();
    int tileWidth = tileMap->GetTileWidth();
    int tileHeight = tileMap->GetTileHeight();
        
    // Render tile layers
    const auto& layers = tileMap->GetLayers();
    for (const auto& li : layers)
    {
        const auto* layer = li.layer;
        if (!layer) continue;
        
        const auto& tiles = layer->getTiles();
        
        for (int y = 0; y < mapHeight; ++y)
        {
            for (int x = 0; x < mapWidth; ++x)
            {
                int index = x + y * mapWidth;
                int gid = tiles[index].ID;
                if (gid == 0) continue;
                
                auto* ts = tileMap->FindTilesetPublic(gid);
                if (!ts) continue;
                
                int id = gid - ts->firstGID;
                
                SDL_Rect src;
                src.x = (id % ts->columns) * ts->tileWidth;
                src.y = (id / ts->columns) * ts->tileHeight;
                src.w = ts->tileWidth;
                src.h = ts->tileHeight;
                
                SDL_Rect dst;
                dst.x = (int)(x * tileWidth + offsetX - cameraX);
                dst.y = y * tileHeight;
                dst.w = tileWidth;
                dst.h = tileHeight;
                
                SDL_RenderCopy(sdl, ts->texture, &src, &dst);
            }
        }
    }
}

void ChunkMap::RenderCollisionBoxes(Renderer* _renderer, Camera* _camera)
{
    SDL_Renderer* sdl = _renderer->GetRenderer();
    float cameraX = _camera ? _camera->GetX() : 0.0f;
    
    Point logicalSize = _renderer->GetLogicalSize();
    int screenWidth = logicalSize.X;
    
    SDL_SetRenderDrawColor(sdl, 255, 0, 0, 255);
    
    for (const auto& chunk : m_activeChunks)
    {
        float chunkRight = chunk.worldOffsetX + m_chunkWidth;
        if (chunkRight < cameraX || chunk.worldOffsetX > cameraX + screenWidth)
            continue;
        
        if (!chunk.tileMap) continue;
        
        const auto& shapes = chunk.tileMap->GetCollisionShapes();
        for (const auto& shape : shapes)
        {
            if (shape.type == CollisionType::Rectangle)
            {
                float shapeX = shape.x + chunk.worldOffsetX;
                float shapeScreenX = _camera ? _camera->WorldToScreenX(shapeX) : shapeX;
                
                SDL_Rect rect;
                rect.x = (int)shapeScreenX;
                rect.y = (int)shape.y;
                rect.w = (int)shape.width;
                rect.h = (int)shape.height;
                
                SDL_RenderDrawRect(sdl, &rect);
            }
        }
    }
}

bool ChunkMap::CheckCollisionTop(float _x, float _y, float _width, float _height, float& _outGroundY) const
{
    float bestY = 100000.0f;
    bool found = false;
    float playerBottom = _y + _height;
    
    for (const auto& chunk : m_activeChunks)
    {
        if (!chunk.tileMap) continue;
        
        const auto& shapes = chunk.tileMap->GetCollisionShapes();
        for (const auto& shape : shapes)
        {
            if (shape.type == CollisionType::Rectangle)
            {
                float shapeX = shape.x + chunk.worldOffsetX;
                float shapeY = shape.y;
                float shapeRight = shapeX + shape.width;
                float shapeBottom = shapeY + shape.height;
                
                // Skip if player is not horizontally overlapping
                if (_x + _width <= shapeX || _x >= shapeRight) continue;
                
                // Ground collision: player's bottom is at or below ground top,
                if (playerBottom >= shapeY && _y < shapeBottom && shapeY < bestY)
                {
                    bestY = shapeY;
                    found = true;
                }
            }
        }
    }
    
    if (found)
    {
        _outGroundY = bestY;
        return true;
    }
    return false;
}

bool ChunkMap::CheckCollisionBottom(float _x, float _y, float _width, float _height, float& _outCeilingY) const
{
    float bestY = -100000.0f;
    bool found = false;
    
    for (const auto& chunk : m_activeChunks)
    {
        if (!chunk.tileMap) continue;
        
        const auto& shapes = chunk.tileMap->GetCollisionShapes();
        for (const auto& shape : shapes)
        {
            if (shape.type == CollisionType::Rectangle)
            {
                float shapeX = shape.x + chunk.worldOffsetX;
                float shapeY = shape.y;
                float shapeRight = shapeX + shape.width;
                float shapeBottom = shapeY + shape.height;
                
                if (_x + _width <= shapeX || _x >= shapeRight) continue;
                
                if (_y <= shapeBottom && _y >= shapeBottom - 5.0f)
                {
                    if (shapeBottom <= _y + _height && shapeBottom > bestY)
                    {
                        bestY = shapeBottom;
                        found = true;
                    }
                }
            }
        }
    }
    
    if (found)
    {
        _outCeilingY = bestY;
        return true;
    }
    return false;
}

bool ChunkMap::CheckCollisionLeft(float _x, float _y, float _width, float _height, float& _outWallX) const
{
    float playerRight = _x + _width;
    float playerBottom = _y + _height;
    float bestX = 100000.0f;
    bool found = false;
    
    for (const auto& chunk : m_activeChunks)
    {
        if (!chunk.tileMap) continue;
        
        const auto& shapes = chunk.tileMap->GetCollisionShapes();
        for (const auto& shape : shapes)
        {
            if (shape.type == CollisionType::Rectangle)
            {
                float shapeX = shape.x + chunk.worldOffsetX;
                float shapeY = shape.y;
                float shapeRight = shapeX + shape.width;
                float shapeBottom = shapeY + shape.height;
                
                // Skip if no vertical overlap
                if (_y + _height <= shapeY || _y >= shapeBottom) continue;
                
                // Only detect as wall if player is NOT standing on top of this shape
                // (i.e., player's bottom is below the shape's top surface)
                bool isStandingOnTop = (playerBottom >= shapeY && playerBottom <= shapeY + 5.0f);
                if (isStandingOnTop) continue;
                
                if (playerRight >= shapeX && playerRight <= shapeX + 5.0f && shapeX < bestX)
                {
                    bestX = shapeX;
                    found = true;
                }
            }
        }
    }
    
    if (found)
    {
        _outWallX = bestX;
        return true;
    }
    return false;
}

bool ChunkMap::CheckCollisionRight(float _x, float _y, float _width, float _height, float& _outWallX) const
{
    float playerLeft = _x;
    float playerBottom = _y + _height;
    float bestX = -100000.0f;
    bool found = false;
    
    for (const auto& chunk : m_activeChunks)
    {
        if (!chunk.tileMap) continue;
        
        const auto& shapes = chunk.tileMap->GetCollisionShapes();
        for (const auto& shape : shapes)
        {
            if (shape.type == CollisionType::Rectangle)
            {
                float shapeX = shape.x + chunk.worldOffsetX;
                float shapeY = shape.y;
                float shapeRight = shapeX + shape.width;
                float shapeBottom = shapeY + shape.height;
                
                // Skip if no vertical overlap
                if (_y + _height <= shapeY || _y >= shapeBottom) continue;
                
                // Only detect as wall if player is NOT standing on top of this shape
                bool isStandingOnTop = (playerBottom >= shapeY && playerBottom <= shapeY + 5.0f);
                if (isStandingOnTop) continue;
                
                if (playerLeft <= shapeRight && playerLeft >= shapeRight - 5.0f && shapeRight > bestX)
                {
                    bestX = shapeRight;
                    found = true;
                }
            }
        }
    }
    
    if (found)
    {
        _outWallX = bestX;
        return true;
    }
    return false;
}

void ChunkMap::Reset()
{
    // Cleanup all entities in active chunks
    for (auto& chunk : m_activeChunks)
    {
        CleanupChunkEntities(chunk);
    }
    
    m_activeChunks.clear();
    
    ChunkInstance startInstance;
    startInstance.tileMap = m_startChunk;
    startInstance.worldOffsetX = 0.0f;
    startInstance.chunkType = 0;
    m_activeChunks.push_back(startInstance);
    
    m_nextChunkX = (float)m_chunkWidth;
}

int ChunkMap::GetChunkPixelWidth() const
{
    return m_chunkWidth;
}

int ChunkMap::GetMapPixelHeight() const
{
    if (m_startChunk)
        return m_startChunk->GetMapPixelHeight();
    return 0;
}

bool ChunkMap::GetPlayerSpawnPoint(float& outX, float& outY) const
{
    if (m_startChunk)
    {
        if (m_startChunk->GetPlayerSpawnPoint(outX, outY))
            return true;
    }
    
    // Default spawn point if none found in map
    outX = 32.0f;
    outY = 0.0f;
    return false;
}

vector<Coin*> ChunkMap::GetAllCoins() const
{
    vector<Coin*> allCoins;
    for (const auto& chunk : m_activeChunks)
    {
        for (auto* coin : chunk.coins)
        {
            if (coin)
                allCoins.push_back(coin);
        }
    }
    return allCoins;
}

vector<Enemy*> ChunkMap::GetAllEnemies() const
{
    vector<Enemy*> allEnemies;
    for (const auto& chunk : m_activeChunks)
    {
        for (auto* enemy : chunk.enemies)
        {
            if (enemy)
                allEnemies.push_back(enemy);
        }
    }
    return allEnemies;
}
