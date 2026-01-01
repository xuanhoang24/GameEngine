#ifndef CHUNK_MAP_H
#define CHUNK_MAP_H

#include "../Core/StandardIncludes.h"
#include "../Graphics/TileMap.h"
#include "../Graphics/Camera.h"
#include <random>

class Renderer;
class Entity;
class EntityManager;

struct BackgroundLayer
{
    SDL_Texture* texture = nullptr;
    int width = 0;
    int height = 0;
    float parallaxFactor = 1.0f;
};

struct ChunkInstance
{
    TileMap* tileMap = nullptr;
    float worldOffsetX = 0.0f;
    int chunkType = 0;
    vector<Entity*> entities;
};

class ChunkMap
{
public:
    ChunkMap();
    virtual ~ChunkMap();

    void SetEntityManager(EntityManager* manager) { m_entityManager = manager; }

    bool Load(const string& _startChunkPath);
    void LoadDefaultChunks();
    void AddRandomChunk(const string& _path);
    void AddGapChunk(const string& _path);
    void AddFloatingChunk(const string& _path);
    
    void AddBackgroundLayer(const string& _path, float _parallaxFactor);
    void RenderBackgrounds(Renderer* _renderer, Camera* _camera);
    
    void Update(float _cameraX, float _screenWidth);
    void Render(Renderer* _renderer, Camera* _camera);
    
    bool CheckCollisionTop(float _x, float _y, float _width, float _height, float& _outGroundY) const;
    bool CheckCollisionBottom(float _x, float _y, float _width, float _height, float& _outCeilingY) const;
    bool CheckCollisionLeft(float _x, float _y, float _width, float _height, float& _outWallX) const;
    bool CheckCollisionRight(float _x, float _y, float _width, float _height, float& _outWallX) const;
    
    bool GetPlayerSpawnPoint(float& outX, float& outY) const;
    void Reset();
    
    int GetChunkPixelWidth() const;
    int GetMapPixelHeight() const;
    
    // Debug rendering for collision shapes
    void RenderCollisionDebug(Renderer* _renderer, Camera* _camera);

private:
    void SpawnNextChunk();
    int SelectRandomChunkType();
    TileMap* SelectRandomChunkVariant(int _type);
    void RenderChunkWithOffset(Renderer* _renderer, Camera* _camera, const ChunkInstance& _chunk);
    void SpawnEntitiesForChunk(ChunkInstance& _chunk);
    void CleanupChunkEntities(ChunkInstance& _chunk);
    
    EntityManager* m_entityManager;
    
    TileMap* m_startChunk;
    vector<TileMap*> m_randomChunks;
    vector<TileMap*> m_gapChunks;
    vector<TileMap*> m_floatingChunks;
    
    vector<ChunkInstance> m_activeChunks;
    vector<BackgroundLayer> m_backgroundLayers;

    float m_nextChunkX;
    int m_chunkWidth;
    
    std::mt19937 m_rng;
    std::uniform_int_distribution<int> m_dist;
    std::uniform_real_distribution<float> m_floatDist;
};

#endif // CHUNK_MAP_H
