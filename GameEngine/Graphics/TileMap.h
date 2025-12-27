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

class TileMap
{
public:
    TileMap();
    virtual ~TileMap();

    bool Load(const string& _path);
    void Render(Renderer* _renderer, Camera* _camera);
    void RenderCollisionBoxes(Renderer* _renderer, Camera* _camera);
    
    // Collision
    bool CheckCollisionTop(float _x, float _y, float _width, float _height, float& _outGroundY) const;
    bool CheckCollisionBottom(float _x, float _y, float _width, float _height, float& _outCeilingY) const;
    bool CheckCollisionLeft(float _x, float _y, float _width, float _height, float& _outWallX) const;
    bool CheckCollisionRight(float _x, float _y, float _width, float _height, float& _outWallX) const;
    
    // Spawn point
    bool GetPlayerSpawnPoint(float& outX, float& outY) const;
    const std::vector<std::pair<float, float>>& GetCoinSpawnPoints() const { return m_coinSpawnPoints; }
    const std::vector<std::pair<float, float>>& GetEnemySpawnPoints() const { return m_enemySpawnPoints; }
    
    // Get enemy zone boundaries (returns left and right X positions for a given spawn point)
    bool GetEnemyZoneBoundaries(float spawnX, float spawnY, float& outLeftX, float& outRightX) const;
    
    // Getters
    int GetMapWidth() const { return m_mapWidth; }
    int GetMapHeight() const { return m_mapHeight; }
    int GetTileWidth() const { return m_tileWidth; }
    int GetTileHeight() const { return m_tileHeight; }
    int GetMapPixelWidth() const { return m_mapWidth * m_tileWidth; }
    int GetMapPixelHeight() const { return m_mapHeight * m_tileHeight; }

private:
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

    // Methods
    void LoadTilesets();
    void LoadLayers(); 
    void LoadImageLayers();
    void LoadCollisionObjects();
    void LoadSpawnPoint();
    void LoadCoinSpawnPoints();
    void LoadEnemySpawnPoints();
    void LoadEnemyZones();
    TilesetInfo* FindTileset(int gid);

    // Members
    tmx::Map m_map;

    std::vector<TilesetInfo> m_tilesets;
    std::vector<LayerInfo>   m_layers;
    std::vector<CollisionShape> m_collisionShapes;
    std::vector<ImageLayerInfo> m_imageLayers;

    // Spawn point
    float m_spawnX = 0.0f;
    float m_spawnY = 0.0f;
    bool m_hasSpawnPoint = false;
    std::vector<std::pair<float, float>> m_coinSpawnPoints;
    std::vector<std::pair<float, float>> m_enemySpawnPoints;
    
    // Enemy zones (left and right boundaries)
    std::vector<std::pair<float, float>> m_enemyZonePoints; // All Enemy_Left and Enemy_Right points

    // Map info
    int m_mapWidth = 0;
    int m_mapHeight = 0;
    int m_tileWidth = 0;
    int m_tileHeight = 0;
};

#endif // TILE_MAP_H

