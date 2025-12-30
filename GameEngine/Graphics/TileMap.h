#ifndef TILE_MAP_H
#define TILE_MAP_H

#include "../Core/StandardIncludes.h"
#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>
#include <tmxlite/ObjectGroup.hpp>
#include <tmxlite/ImageLayer.hpp>
#include <tmxlite/LayerGroup.hpp>
#include "../Graphics/Renderer.h"
#include "../Graphics/Texture.h"
#include "../Graphics/Camera.h"
#include "CollisionShape.h"

// Spawn zone for coins
struct CoinSpawnZone
{
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    float chance = 1.0f;        // Probability of spawning (0.0 - 1.0)
    int minCount = 1;
    int maxCount = 1;
};

// Spawn zone for enemies
struct EnemySpawnZone
{
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    float chance = 1.0f;        // Probability of spawning (0.0 - 1.0)
    int maxCount = 1;
    vector<string> enemyTypes;  // e.g., "ghost", "mushroom"
    vector<float> enemyWeights; // Weights for each type
};

class TileMap
{
public:
    TileMap();
    virtual ~TileMap();

    bool Load(const string& _path);
    void Render(Renderer* _renderer, Camera* _camera);
    
    // Collision
    bool CheckCollisionTop(float _x, float _y, float _width, float _height, float& _outGroundY) const;
    bool CheckCollisionBottom(float _x, float _y, float _width, float _height, float& _outCeilingY) const;
    bool CheckCollisionLeft(float _x, float _y, float _width, float _height, float& _outWallX) const;
    bool CheckCollisionRight(float _x, float _y, float _width, float _height, float& _outWallX) const;
    
    // Spawn point
    bool GetPlayerSpawnPoint(float& outX, float& outY) const;
    bool GetEndPoint(float& outX, float& outY) const;
    
    // Getters
    int GetMapWidth() const { return m_mapWidth; }
    int GetMapHeight() const { return m_mapHeight; }
    int GetTileWidth() const { return m_tileWidth; }
    int GetTileHeight() const { return m_tileHeight; }
    int GetMapPixelWidth() const { return m_mapWidth * m_tileWidth; }
    int GetMapPixelHeight() const { return m_mapHeight * m_tileHeight; }

public:
    struct TilesetInfo
    {
        SDL_Texture* texture = nullptr;
        int firstGID = 0;

        int tileWidth = 0;
        int tileHeight = 0;

        int columns = 0;
        int imageWidth = 0;
        int imageHeight = 0;
    };

    struct ImageLayerInfo
    {
        SDL_Texture* texture = nullptr;
        int x = 0;
        int y = 0;
    };

    struct LayerInfo
    {
        const tmx::TileLayer* layer = nullptr;
    };
    
    // Public accessors for ChunkMap
    const vector<TilesetInfo>& GetTilesets() const { return m_tilesets; }
    const vector<LayerInfo>& GetLayers() const { return m_layers; }
    const vector<ImageLayerInfo>& GetImageLayers() const { return m_imageLayers; }
    const vector<CollisionShape>& GetCollisionShapes() const { return m_collisionShapes; }
    const vector<CoinSpawnZone>& GetCoinSpawnZones() const { return m_coinSpawnZones; }
    const vector<EnemySpawnZone>& GetEnemySpawnZones() const { return m_enemySpawnZones; }
    TilesetInfo* FindTilesetPublic(int gid) { return FindTileset(gid); }

private:

    // Methods
    void LoadTilesets();
    void LoadLayers(); 
    void LoadImageLayers();
    void LoadCollisionObjects();
    void LoadSpawnPoint();
    void LoadSpawnZones();
    TilesetInfo* FindTileset(int gid);

    // Members
    tmx::Map m_map;

    std::vector<TilesetInfo> m_tilesets;
    std::vector<LayerInfo>   m_layers;
    std::vector<CollisionShape> m_collisionShapes;
    std::vector<ImageLayerInfo> m_imageLayers;
    std::vector<CoinSpawnZone> m_coinSpawnZones;
    std::vector<EnemySpawnZone> m_enemySpawnZones;

    // Spawn point
    float m_startX = 0.0f;
    float m_startY = 0.0f;
    bool m_hasStartPoint = false;
    
    // End point (for chunk connections)
    float m_endX = 0.0f;
    float m_endY = 0.0f;
    bool m_hasEndPoint = false;

    // Map info
    int m_mapWidth = 0;
    int m_mapHeight = 0;
    int m_tileWidth = 0;
    int m_tileHeight = 0;
};

#endif // TILE_MAP_H

