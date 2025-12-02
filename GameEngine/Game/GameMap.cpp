#include "../Game/GameMap.h"

GameMap::GameMap()
{
    m_tileMap = nullptr;
}

GameMap::~GameMap()
{
    delete m_tileMap;
    m_tileMap = nullptr;
}

bool GameMap::Load(const string& _file)
{
    m_tileMap = new TileMap();
    return m_tileMap->Load(_file);
}

void GameMap::Render(Renderer* _renderer)
{
    if (m_tileMap)
        m_tileMap->Render(_renderer);
}

bool GameMap::CheckCollision(float _x, float _y, float _width, float _height) const
{
    if (m_tileMap)
        return m_tileMap->CheckCollision(_x, _y, _width, _height);
    return false;
}

float GameMap::GetGroundY(float _x, float _y, float _width, float _height) const
{
    if (m_tileMap)
        return m_tileMap->GetGroundY(_x, _y, _width, _height);
    return 10000.0f;
}

int GameMap::GetMapPixelWidth() const
{
    if (m_tileMap)
        return m_tileMap->GetMapPixelWidth();
    return 0;
}

int GameMap::GetMapPixelHeight() const
{
    if (m_tileMap)
        return m_tileMap->GetMapPixelHeight();
    return 0;
}

void GameMap::SetMapYOffset(int _yOffset)
{
    if (m_tileMap)
        m_tileMap->SetYOffset(_yOffset);
}
