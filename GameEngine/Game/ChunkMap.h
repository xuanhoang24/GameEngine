#ifndef CHUNK_MAP_H
#define CHUNK_MAP_H

#include "../Core/StandardIncludes.h"
#include "../Graphics/TileMap.h"
#include "../Graphics/Camera.h"
#include <random>

class Renderer;

// Represents a single chunk instance in the world
struct ChunkInstance
{
    TileMap* tileMap = nullptr;
    float worldOffsetX = 0.0f;
    int chunkType = 0;          // 0 = start, 1 = random, 2 = gap
};

class ChunkMap
{
public:
    ChunkMap();
    virtual ~ChunkMap();

    bool Load(const string& _startChunkPath, const string& _randomChunkPath, const string& _gapChunkPath);
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

private:
    void SpawnNextChunk();
    int SelectRandomChunkType();  // Returns 1 (random 70%) or 2 (gap 30%)
    void RenderChunkWithOffset(Renderer* _renderer, Camera* _camera, const ChunkInstance& _chunk);
    
    // Template chunks (loaded once, reused)
    TileMap* m_startChunk;
    TileMap* m_randomChunk;
    TileMap* m_gapChunk;
    
    // Active chunk instances
    vector<ChunkInstance> m_activeChunks;
    
    // Chunk paths for reference
    string m_startChunkPath;
    string m_randomChunkPath;
    string m_gapChunkPath;
    
    // World state
    float m_nextChunkX;         // X position where next chunk should spawn
    int m_chunkWidth;           // Width of each chunk in pixels
    
    // Random number generator
    std::mt19937 m_rng;
    std::uniform_int_distribution<int> m_dist;
};

#endif // CHUNK_MAP_H
