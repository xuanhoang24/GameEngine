#ifndef GAME_MAP_H
#define GAME_MAP_H

#include "../Core/StandardIncludes.h"
#include "../Graphics/TileMap.h"
#include "../Graphics/Camera.h"

class Renderer;

class GameMap
{
public:
    GameMap();
    ~GameMap();

    bool Load(const string& _file);
    void Render(Renderer* _renderer, Camera* _camera);
    
    // Collision
    bool CheckGround(float _x, float _y, float _width, float _height, float& _outGroundY) const;
    
    // Getters
    int GetMapPixelWidth() const;
    int GetMapPixelHeight() const;
    
    // Setters
    void SetMapYOffset(int _yOffset);

private:
    TileMap* m_tileMap = nullptr;
};

#endif // GAME_MAP_H
