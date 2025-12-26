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
    void RenderCollisionBoxes(Renderer* _renderer, Camera* _camera);
    
    // Collision
    bool CheckCollisionTop(float _x, float _y, float _width, float _height, float& _outGroundY) const;
    bool CheckCollisionBottom(float _x, float _y, float _width, float _height, float& _outCeilingY) const;
    bool CheckCollisionLeft(float _x, float _y, float _width, float _height, float& _outWallX) const;
    bool CheckCollisionRight(float _x, float _y, float _width, float _height, float& _outWallX) const;
    
    // Spawn point
    bool GetPlayerSpawnPoint(float& outX, float& outY) const;
    
    // Getters
    int GetMapPixelWidth() const;
    int GetMapPixelHeight() const;

private:
    TileMap* m_tileMap = nullptr;
};

#endif // GAME_MAP_H
