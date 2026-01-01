#include "../Game/ChunkMap.h"
#include "../Graphics/Renderer.h"
#include "../Game/EntityManager.h"
#include "../Core/Timing.h"

ChunkMap::ChunkMap()
    : m_entityManager(nullptr)
    , m_startChunk(nullptr)
    , m_nextChunkX(0.0f)
    , m_chunkWidth(0)
    , m_rng(std::random_device{}())
    , m_dist(1, 100)
    , m_floatDist(0.0f, 1.0f)
{
}

ChunkMap::~ChunkMap()
{
    for (auto& chunk : m_activeChunks)
        CleanupChunkEntities(chunk);
    
    for (auto& bg : m_backgroundLayers)
        if (bg.texture) SDL_DestroyTexture(bg.texture);
    m_backgroundLayers.clear();
    
    delete m_startChunk;
    for (auto* chunk : m_randomChunks) delete chunk;
    for (auto* chunk : m_gapChunks) delete chunk;
    for (auto* chunk : m_floatingChunks) delete chunk;
    
    m_randomChunks.clear();
    m_gapChunks.clear();
    m_floatingChunks.clear();
    m_activeChunks.clear();
}

bool ChunkMap::Load(const string& _startChunkPath)
{
    m_startChunk = new TileMap();
    if (!m_startChunk->Load(_startChunkPath))
        return false;
    
    m_chunkWidth = m_startChunk->GetMapPixelWidth();
    
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
    if (chunk->Load(_path)) m_randomChunks.push_back(chunk);
    else delete chunk;
}

void ChunkMap::AddGapChunk(const string& _path)
{
    TileMap* chunk = new TileMap();
    if (chunk->Load(_path)) m_gapChunks.push_back(chunk);
    else delete chunk;
}

void ChunkMap::AddFloatingChunk(const string& _path)
{
    TileMap* chunk = new TileMap();
    if (chunk->Load(_path)) m_floatingChunks.push_back(chunk);
    else delete chunk;
}

void ChunkMap::AddBackgroundLayer(const string& _path, float _parallaxFactor)
{
    SDL_Renderer* sdl = Renderer::Instance().GetRenderer();
    SDL_Texture* texture = IMG_LoadTexture(sdl, _path.c_str());
    
    if (texture)
    {
        BackgroundLayer layer;
        layer.texture = texture;
        layer.parallaxFactor = _parallaxFactor;
        SDL_QueryTexture(texture, nullptr, nullptr, &layer.width, &layer.height);
        m_backgroundLayers.push_back(layer);
    }
}

void ChunkMap::RenderBackgrounds(Renderer* _renderer, Camera* _camera)
{
    SDL_Renderer* sdl = _renderer->GetRenderer();
    float cameraX = _camera ? _camera->GetX() : 0.0f;
    
    Point logicalSize = _renderer->GetLogicalSize();
    int screenWidth = logicalSize.X;
    int screenHeight = logicalSize.Y;
    
    for (const auto& layer : m_backgroundLayers)
    {
        if (!layer.texture || layer.width == 0) continue;
        
        float scale = (float)screenHeight / (float)layer.height;
        int scaledWidth = (int)(layer.width * scale);
        int scaledHeight = screenHeight;
        
        if (scaledWidth == 0) continue;
        
        float parallaxOffset = cameraX * layer.parallaxFactor;
        float offset = fmod(parallaxOffset, (float)scaledWidth);
        float startX = -offset;
        
        while (startX > 0) startX -= scaledWidth;
        
        for (float x = startX; x < screenWidth + scaledWidth; x += scaledWidth)
        {
            SDL_Rect dst = { (int)x, 0, scaledWidth, scaledHeight };
            SDL_RenderCopy(sdl, layer.texture, nullptr, &dst);
        }
    }
}

void ChunkMap::LoadDefaultChunks()
{
    AddBackgroundLayer("../Assets/Maps/Images/Tilemap/Background_2.png", 0.3f);
    AddBackgroundLayer("../Assets/Maps/Images/Tilemap/Background_1.png", 0.6f);
    
    Load("../Assets/Maps/Chunk/chunk_flat_start.tmx");
    
    AddRandomChunk("../Assets/Maps/Chunk/chunk_random_01.tmx");
    AddRandomChunk("../Assets/Maps/Chunk/chunk_random_11.tmx");
    AddRandomChunk("../Assets/Maps/Chunk/chunk_random_21.tmx");
    
    AddGapChunk("../Assets/Maps/Chunk/chunk_gap_01.tmx");
    
    AddFloatingChunk("../Assets/Maps/Chunk/chunk_floating_01.tmx");
    AddFloatingChunk("../Assets/Maps/Chunk/chunk_floating_02.tmx");
    AddFloatingChunk("../Assets/Maps/Chunk/chunk_floating_11.tmx");
    AddFloatingChunk("../Assets/Maps/Chunk/chunk_floating_12.tmx");
}

void ChunkMap::Update(float _cameraX, float _screenWidth)
{
    float spawnThreshold = _cameraX + _screenWidth + m_chunkWidth;
    
    while (m_nextChunkX < spawnThreshold)
        SpawnNextChunk();
    
    float despawnThreshold = _cameraX - m_chunkWidth * 2;
    
    auto it = m_activeChunks.begin();
    while (it != m_activeChunks.end())
    {
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
}

void ChunkMap::SpawnNextChunk()
{
    ChunkInstance newChunk;
    newChunk.worldOffsetX = m_nextChunkX;
    newChunk.chunkType = SelectRandomChunkType();
    newChunk.tileMap = SelectRandomChunkVariant(newChunk.chunkType);
    
    if (newChunk.tileMap)
    {
        SpawnEntitiesForChunk(newChunk);
        m_activeChunks.push_back(newChunk);
    }
    
    m_nextChunkX += m_chunkWidth;
}

int ChunkMap::SelectRandomChunkType()
{
    int roll = m_dist(m_rng);
    if (roll <= 30) return 1;
    else if (roll <= 60) return 2;
    else if (roll <= 90) return 3;
    else return 1;
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
        if (!m_randomChunks.empty()) chunks = &m_randomChunks;
        else return nullptr;
    }
    
    std::uniform_int_distribution<size_t> dist(0, chunks->size() - 1);
    return (*chunks)[dist(m_rng)];
}

void ChunkMap::SpawnEntitiesForChunk(ChunkInstance& _chunk)
{
    if (!_chunk.tileMap || !m_entityManager) return;
    
    // Spawn coins
    const auto& coinZones = _chunk.tileMap->GetCoinSpawnZones();
    for (const auto& zone : coinZones)
    {
        if (m_floatDist(m_rng) > zone.chance) continue;
        
        int count = zone.minCount;
        if (zone.maxCount > zone.minCount)
        {
            std::uniform_int_distribution<int> countDist(zone.minCount, zone.maxCount);
            count = countDist(m_rng);
        }
        
        for (int i = 0; i < count; ++i)
        {
            float coinWidth = 16.0f;
            float coinHeight = 16.0f;
            std::uniform_real_distribution<float> xDist(zone.x + coinWidth * 0.5f, zone.x + zone.width - coinWidth * 0.5f);
            std::uniform_real_distribution<float> yDist(zone.y + coinHeight, zone.y + zone.height);
            
            float localX = xDist(m_rng);
            float localY = yDist(m_rng);
            float worldX = localX + _chunk.worldOffsetX;
            float worldY = localY;
            
            Entity* coin = EntityFactory::CreateRandomCoin(worldX, worldY);
            m_entityManager->GetAllEntities().push_back(coin);
            _chunk.entities.push_back(coin);
        }
    }
    
    // Spawn enemies
    const auto& enemyZones = _chunk.tileMap->GetEnemySpawnZones();
    for (const auto& zone : enemyZones)
    {
        if (m_floatDist(m_rng) > zone.chance) continue;
        
        for (int i = 0; i < zone.maxCount; ++i)
        {
            float enemyWidth = 16.0f;
            std::uniform_real_distribution<float> xDist(zone.x + enemyWidth * 0.5f, zone.x + zone.width - enemyWidth * 0.5f);
            
            float localX = xDist(m_rng);
            float localY = zone.y + zone.height;
            float worldX = localX + _chunk.worldOffsetX;
            float worldY = localY;
            
            EnemyVariant enemyVariant = EnemyVariant::Ghost;
            if (!zone.enemyTypes.empty() && !zone.enemyWeights.empty())
            {
                float weightRoll = m_floatDist(m_rng);
                float cumulative = 0.0f;
                for (size_t j = 0; j < zone.enemyTypes.size(); ++j)
                {
                    cumulative += (j < zone.enemyWeights.size()) ? zone.enemyWeights[j] : 0.5f;
                    if (weightRoll <= cumulative)
                    {
                        if (zone.enemyTypes[j] == "mushroom")
                            enemyVariant = EnemyVariant::Mushroom;
                        break;
                    }
                }
            }
            
            float leftBound = zone.x + _chunk.worldOffsetX;
            float rightBound = zone.x + zone.width + _chunk.worldOffsetX - enemyWidth;
            
            Entity* enemy = EntityFactory::CreateEnemy(worldX, worldY, enemyVariant, leftBound, rightBound);
            m_entityManager->GetAllEntities().push_back(enemy);
            _chunk.entities.push_back(enemy);
        }
    }
}

void ChunkMap::CleanupChunkEntities(ChunkInstance& _chunk)
{
    if (!m_entityManager) return;
    
    for (auto* entity : _chunk.entities)
    {
        if (entity)
            m_entityManager->DestroyEntity(entity);
    }
    _chunk.entities.clear();
}

void ChunkMap::Render(Renderer* _renderer, Camera* _camera)
{
    float cameraX = _camera ? _camera->GetX() : 0.0f;
    Point logicalSize = _renderer->GetLogicalSize();
    int screenWidth = logicalSize.X;
    
    for (const auto& chunk : m_activeChunks)
    {
        float chunkRight = chunk.worldOffsetX + m_chunkWidth;
        if (chunkRight < cameraX || chunk.worldOffsetX > cameraX + screenWidth)
            continue;
        
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
                
                if (_x + _width <= shapeX || _x >= shapeRight) continue;
                
                if (playerBottom >= shapeY && _y < shapeBottom && shapeY < bestY)
                {
                    bestY = shapeY;
                    found = true;
                }
            }
        }
    }
    
    if (found) { _outGroundY = bestY; return true; }
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
    
    if (found) { _outCeilingY = bestY; return true; }
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
                
                if (_y + _height <= shapeY || _y >= shapeBottom) continue;
                
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
    
    if (found) { _outWallX = bestX; return true; }
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
                
                if (_y + _height <= shapeY || _y >= shapeBottom) continue;
                
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
    
    if (found) { _outWallX = bestX; return true; }
    return false;
}

void ChunkMap::Reset()
{
    for (auto& chunk : m_activeChunks)
        CleanupChunkEntities(chunk);
    
    m_activeChunks.clear();
    
    ChunkInstance startInstance;
    startInstance.tileMap = m_startChunk;
    startInstance.worldOffsetX = 0.0f;
    startInstance.chunkType = 0;
    m_activeChunks.push_back(startInstance);
    
    m_nextChunkX = (float)m_chunkWidth;
}

int ChunkMap::GetChunkPixelWidth() const { return m_chunkWidth; }

int ChunkMap::GetMapPixelHeight() const
{
    return m_startChunk ? m_startChunk->GetMapPixelHeight() : 0;
}

bool ChunkMap::GetPlayerSpawnPoint(float& outX, float& outY) const
{
    if (m_startChunk && m_startChunk->GetPlayerSpawnPoint(outX, outY))
        return true;
    
    outX = 32.0f;
    outY = 0.0f;
    return false;
}

void ChunkMap::RenderCollisionDebug(Renderer* _renderer, Camera* _camera)
{
    if (!_renderer) return;
    
    float cameraX = _camera ? _camera->GetX() : 0.0f;
    Point logicalSize = _renderer->GetLogicalSize();
    int screenWidth = logicalSize.X;
    
    // Blue color for map collision shapes
    _renderer->SetDrawColor(Color(0, 0, 255, 255));
    
    for (const auto& chunk : m_activeChunks)
    {
        if (!chunk.tileMap) continue;
        
        // Skip chunks not visible on screen
        float chunkRight = chunk.worldOffsetX + m_chunkWidth;
        if (chunkRight < cameraX || chunk.worldOffsetX > cameraX + screenWidth)
            continue;
        
        const auto& shapes = chunk.tileMap->GetCollisionShapes();
        for (const auto& shape : shapes)
        {
            if (shape.type == CollisionType::Rectangle)
            {
                float worldX = shape.x + chunk.worldOffsetX;
                float worldY = shape.y;
                float screenX = worldX - cameraX;
                float screenY = worldY;
                
                Rect collisionRect(
                    static_cast<unsigned int>(screenX),
                    static_cast<unsigned int>(screenY),
                    static_cast<unsigned int>(screenX + shape.width),
                    static_cast<unsigned int>(screenY + shape.height)
                );
                _renderer->RenderRectangle(collisionRect);
            }
        }
    }
}
