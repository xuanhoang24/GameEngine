#include "../Game/ChunkMap.h"
#include "../Graphics/Renderer.h"

ChunkMap::ChunkMap()
    : m_startChunk(nullptr)
    , m_randomChunk(nullptr)
    , m_gapChunk(nullptr)
    , m_nextChunkX(0.0f)
    , m_chunkWidth(0)
    , m_rng(std::random_device{}())
    , m_dist(1, 100)
{
}

ChunkMap::~ChunkMap()
{
    delete m_startChunk;
    delete m_randomChunk;
    delete m_gapChunk;
    m_activeChunks.clear();
}

bool ChunkMap::Load(const string& _startChunkPath, const string& _randomChunkPath, const string& _gapChunkPath)
{
    m_startChunkPath = _startChunkPath;
    m_randomChunkPath = _randomChunkPath;
    m_gapChunkPath = _gapChunkPath;
    
    // Load template chunks
    m_startChunk = new TileMap();
    if (!m_startChunk->Load(_startChunkPath))
        return false;
    
    m_randomChunk = new TileMap();
    if (!m_randomChunk->Load(_randomChunkPath))
        return false;
    
    m_gapChunk = new TileMap();
    if (!m_gapChunk->Load(_gapChunkPath))
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
    
    // Point to the appropriate template
    if (newChunk.chunkType == 1)
        newChunk.tileMap = m_randomChunk;
    else
        newChunk.tileMap = m_gapChunk;
    
    m_activeChunks.push_back(newChunk);
    m_nextChunkX += m_chunkWidth;
}

int ChunkMap::SelectRandomChunkType()
{
    int roll = m_dist(m_rng);
    // 70% chance for random chunk (type 1), 30% for gap chunk (type 2)
    return (roll <= 70) ? 1 : 2;
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
