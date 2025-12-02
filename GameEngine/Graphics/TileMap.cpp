#include "../Graphics/TileMap.h"

TileMap::TileMap() 
{
}

TileMap::~TileMap()
{
    for (auto& ts : m_tilesets)
        if (ts.texture)
            SDL_DestroyTexture(ts.texture);
}

bool TileMap::Load(const string& _path)
{
    M_ASSERT(m_map.load(_path), "Failed to load TMX")

    // Extract map size
    m_tileWidth = m_map.getTileSize().x;
    m_tileHeight = m_map.getTileSize().y;
    m_mapWidth = m_map.getTileCount().x;
    m_mapHeight = m_map.getTileCount().y;

    // Load
    LoadTilesets();
    LoadLayers();

    return true;
}

void TileMap::LoadTilesets()
{
    const auto& sets = m_map.getTilesets();

    for (const auto& ts : sets)
    {
        TilesetInfo info;

        info.firstGID = ts.getFirstGID();
        info.tileWidth = ts.getTileSize().x;
        info.tileHeight = ts.getTileSize().y;
        info.imageWidth = ts.getImageSize().x;
        info.imageHeight = ts.getImageSize().y;
        info.columns = ts.getColumnCount();

        // Tileset PNG path
        string imgPath = ts.getImagePath();

        SDL_Surface* surf = IMG_Load(imgPath.c_str());
        M_ASSERT(surf != nullptr, "Failed to load tileset PNG");

        SDL_Renderer* sdl = Renderer::Instance().GetRenderer();
        info.texture = SDL_CreateTextureFromSurface(sdl, surf);
        SDL_FreeSurface(surf);
        M_ASSERT(info.texture != nullptr, "Failed to create SDL texture");

        m_tilesets.push_back(info);
    }
}

void TileMap::LoadLayers()
{
    for (auto& layer : m_map.getLayers())
    {
        if (layer->getType() != tmx::Layer::Type::Tile)
            continue;

        LayerInfo li;
        li.layer = &layer->getLayerAs<tmx::TileLayer>();

        m_layers.push_back(li);
    }
}

TileMap::TilesetInfo* TileMap::FindTileset(int _gid)
{
    if (_gid == 0) return nullptr;

    TilesetInfo* result = nullptr;

    for (auto& ts : m_tilesets)
    {
        if (_gid >= ts.firstGID)
        {
            if (!result || ts.firstGID > result->firstGID)
                result = &ts;
        }
    }
    return result;
}

void TileMap::Render(Renderer* _renderer)
{
    SDL_Renderer* sdl = _renderer->GetRenderer();

    for (auto& li : m_layers)
    {
        const auto* layer = li.layer;
        if (!layer) continue;

        const auto& tiles = layer->getTiles();

        for (int y = 0; y < m_mapHeight; ++y)
        {
            for (int x = 0; x < m_mapWidth; ++x)
            {
                int index = x + y * m_mapWidth;
                int gid = tiles[index].ID;
                if (gid == 0) continue;

                TilesetInfo* ts = FindTileset(gid);
                if (!ts) continue;

                int id = gid - ts->firstGID;

                SDL_Rect src;
                src.x = (id % ts->columns) * ts->tileWidth;
                src.y = (id / ts->columns) * ts->tileHeight;
                src.w = ts->tileWidth;
                src.h = ts->tileHeight;

                SDL_Rect dst;
                dst.x = x * m_tileWidth;
                dst.y = y * m_tileHeight + m_yOffset;
                dst.w = m_tileWidth;
                dst.h = m_tileHeight;

                SDL_RenderCopy(sdl, ts->texture, &src, &dst);
            }
        }
    }
}

bool TileMap::IsTileSolid(int _tileX, int _tileY) const
{
    // Out of bounds check
    if (_tileX < 0 || _tileX >= m_mapWidth || _tileY < 0 || _tileY >= m_mapHeight)
        return false;

    // Check all layers for solid tiles
    for (const auto& li : m_layers)
    {
        const auto* layer = li.layer;
        if (!layer) continue;

        const auto& tiles = layer->getTiles();
        int index = _tileX + _tileY * m_mapWidth;
        
        if (index >= 0 && index < tiles.size())
        {
            int gid = tiles[index].ID;
            // Any tile with a GID > 0 is considered solid (temp)
            if (gid > 0)
                return true;
        }
    }
    
    return false;
}

bool TileMap::CheckCollision(float _x, float _y, float _width, float _height) const
{
    // Convert world coordinates to tile coordinates (accounting for offset)
    float adjustedY = _y - m_yOffset;
    
    int startX = (int)(_x / m_tileWidth);
    int endX = (int)((_x + _width) / m_tileWidth);
    int startY = (int)(adjustedY / m_tileHeight);
    int endY = (int)((adjustedY + _height) / m_tileHeight);

    // Check all tiles in the bounding box
    for (int y = startY; y <= endY; ++y)
    {
        for (int x = startX; x <= endX; ++x)
        {
            if (IsTileSolid(x, y))
                return true;
        }
    }
    
    return false;
}

float TileMap::GetGroundY(float _x, float _y, float _width, float _height) const
{
    // Convert world coordinates to tile coordinates
    float adjustedY = _y - m_yOffset;
    
    int startX = (int)(_x / m_tileWidth);
    int endX = (int)((_x + _width) / m_tileWidth);
    int checkY = (int)((adjustedY + _height) / m_tileHeight);

    // Check tiles below the player
    for (int x = startX; x <= endX; ++x)
    {
        if (IsTileSolid(x, checkY))
        {
            // Return the top of the tile in world coordinates
            return (checkY * m_tileHeight) + m_yOffset;
        }
    }
    
    // No ground found, return a default far below
    return 10000.0f;
}
