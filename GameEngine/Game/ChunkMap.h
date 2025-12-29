#ifndef CHUNK_MAP_H
#define CHUNK_MAP_H

#include "../Core/StandardIncludes.h"
#include "../Graphics/TileMap.h"
#include "../Graphics/Camera.h"
#include <random>

class Renderer;
class Coin;
class Enemy;

// Represents a single chunk instance in the world
struct ChunkInstance
{
    TileMap* tileMap = nullptr;
    float worldOffsetX = 0.0f;
    int chunkType = 0;          // 0 = start, 1 = random, 2 = gap, 3 = floating
    vector<Coin*> coins;        // Coins spawned in this chunk
    vector<Enemy*> enemies;     // Enemies spawned in this chunk
};

class ChunkMap
{
public:
    ChunkMap();
    virtual ~ChunkMap();

    bool Load(const string& _startChunkPath);
    void LoadDefaultChunks();
    void AddRandomChunk(const string& _path);
    void AddGapChunk(const string& _path);
    void AddFloatingChunk(const string& _path);
    
    void Update(float _cameraX, float _screenWidth);
    void Render(Renderer* _renderer, Camera* _camera);
    void RenderCollisionBoxes(Renderer* _renderer, Camera* _camera);
    
    // Collision - checks all active chunks
    bool CheckCollisionTop(float _x, float _y, float _width, float _height, float& _outGroundY) const;
    bool CheckCollisionBottom(float _x, float _y, float _width, float _height, float& _outCeilingY) const;
    bool CheckCollisionLeft(float _x, float _y, float _width, float _height, float& _outWallX) const;
    bool CheckCollisionRight(float _x, float _y, float _width, float _height, float& _outWallX) const;
    
    // Spawn point
    bool GetPlayerSpawnPoint(float& outX, float& outY) const;
    
    // Reset for game restart
    void Reset();
    
    // Getters
    int GetChunkPixelWidth() const;
    int GetMapPixelHeight() const;
    
    // Get all active coins and enemies
    vector<Coin*> GetAllCoins() const;
    vector<Enemy*> GetAllEnemies() const;

private:
    void SpawnNextChunk();
    int SelectRandomChunkType();  // Returns 1 (random), 2 (gap), or 3 (floating)
    TileMap* SelectRandomChunkVariant(int _type);
    void RenderChunkWithOffset(Renderer* _renderer, Camera* _camera, const ChunkInstance& _chunk);
    void SpawnEntitiesForChunk(ChunkInstance& _chunk);
    void CleanupChunkEntities(ChunkInstance& _chunk);
    
    // Template chunks (loaded once, reused)
    TileMap* m_startChunk;
    vector<TileMap*> m_randomChunks;
    vector<TileMap*> m_gapChunks;
    vector<TileMap*> m_floatingChunks;
    
    // Active chunk instances
    vector<ChunkInstance> m_activeChunks;
    
    // World state
    float m_nextChunkX;         // X position where next chunk should spawn
    int m_chunkWidth;           // Width of each chunk in pixels
    
    // Random number generator
    std::mt19937 m_rng;
    std::uniform_int_distribution<int> m_dist;
    std::uniform_real_distribution<float> m_floatDist;
};

#endif // CHUNK_MAP_H
