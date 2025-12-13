#ifndef TILE_MAP_H
#define TILE_MAP_H

#include "../Core/StandardIncludes.h"
#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>
#include <tmxlite/ObjectGroup.hpp>
#include <tmxlite/ImageLayer.hpp>
#include "../Graphics/Renderer.h"
#include "../Graphics/Texture.h"
#include "CollisionShape.h"

class TileMap
{
public:
    TileMap();
    virtual ~TileMap();

    bool Load(const string& _path);
    void Render(Renderer* _renderer);
    
    // Collision
    bool CheckGroundCollision(float _x, float _y, float _width, float _height, float& _outGroundY) const;
    
    // Getters
    int GetMapWidth() const { return m_mapWidth; }
    int GetMapHeight() const { return m_mapHeight; }
    int GetTileWidth() const { return m_tileWidth; }
    int GetTileHeight() const { return m_tileHeight; }
    int GetMapPixelWidth() const { return m_mapWidth * m_tileWidth; }
    int GetMapPixelHeight() const { return m_mapHeight * m_tileHeight; }
    int GetYOffset() const { return m_yOffset; }

    // Setters
    void SetYOffset(int _yOffset) { m_yOffset = _yOffset; }

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
    TilesetInfo* FindTileset(int gid);

    // Members
    tmx::Map m_map;

    std::vector<TilesetInfo> m_tilesets;
    std::vector<LayerInfo>   m_layers;
    std::vector<CollisionShape> m_collisionShapes;
    std::vector<ImageLayerInfo> m_imageLayers;

    // Map info
    int m_mapWidth = 0;
    int m_mapHeight = 0;
    int m_tileWidth = 0;
    int m_tileHeight = 0;
    int m_yOffset = 0;
};

#endif // TILE_MAP_H

