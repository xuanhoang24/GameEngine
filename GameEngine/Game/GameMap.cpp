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

void GameMap::Render(Renderer* _renderer, Camera* _camera)
{
    if (m_tileMap)
        m_tileMap->Render(_renderer, _camera);
}

void GameMap::RenderCollisionBoxes(Renderer* _renderer, Camera* _camera)
{
    if (m_tileMap)
        m_tileMap->RenderCollisionBoxes(_renderer, _camera);
}

bool GameMap::CheckCollisionTop(float _x, float _y, float _width, float _height, float& _outGroundY) const
{
    if (!m_tileMap) return false;
    return m_tileMap->CheckCollisionTop(_x, _y, _width, _height, _outGroundY);
}

bool GameMap::CheckCollisionBottom(float _x, float _y, float _width, float _height, float& _outCeilingY) const
{
    if (!m_tileMap) return false;
    return m_tileMap->CheckCollisionBottom(_x, _y, _width, _height, _outCeilingY);
}

bool GameMap::CheckCollisionLeft(float _x, float _y, float _width, float _height, float& _outWallX) const
{
    if (!m_tileMap) return false;
    return m_tileMap->CheckCollisionLeft(_x, _y, _width, _height, _outWallX);
}

bool GameMap::CheckCollisionRight(float _x, float _y, float _width, float _height, float& _outWallX) const
{
    if (!m_tileMap) return false;
    return m_tileMap->CheckCollisionRight(_x, _y, _width, _height, _outWallX);
}

bool GameMap::GetPlayerSpawnPoint(float& outX, float& outY) const
{
    if (!m_tileMap) return false;
    return m_tileMap->GetPlayerSpawnPoint(outX, outY);
}

const vector<pair<float, float>>& GameMap::GetCoinSpawnPoints() const
{
    static vector<pair<float, float>> empty;
    if (!m_tileMap) return empty;
    return m_tileMap->GetCoinSpawnPoints();
}

const vector<pair<float, float>>& GameMap::GetEnemySpawnPoints() const
{
    static vector<pair<float, float>> empty;
    if (!m_tileMap) return empty;
    return m_tileMap->GetEnemySpawnPoints();
}

bool GameMap::GetEnemyZoneBoundaries(float spawnX, float spawnY, float& outLeftX, float& outRightX) const
{
    if (!m_tileMap) return false;
    return m_tileMap->GetEnemyZoneBoundaries(spawnX, spawnY, outLeftX, outRightX);
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
