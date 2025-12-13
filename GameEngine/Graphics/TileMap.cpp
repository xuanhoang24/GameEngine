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
    M_ASSERT(m_map.load(_path), "Failed to load TMX");

    // Extract map size
    m_tileWidth = m_map.getTileSize().x;
    m_tileHeight = m_map.getTileSize().y;
    m_mapWidth = m_map.getTileCount().x;
    m_mapHeight = m_map.getTileCount().y;

    // Load
    LoadTilesets();
    LoadImageLayers();
    LoadLayers();
    LoadCollisionObjects();

    return true;
}

void TileMap::LoadTilesets()
{
    const auto& sets = m_map.getTilesets();

    for (const auto& ts : sets)
    {
        TilesetInfo info;
        info.firstGID   = ts.getFirstGID();
        info.tileWidth  = ts.getTileSize().x;
        info.tileHeight = ts.getTileSize().y;
        info.columns    = ts.getColumnCount();
        info.imageWidth = ts.getImageSize().x;
        info.imageHeight = ts.getImageSize().y;

        string imgPath = ts.getImagePath();
        SDL_Surface* surf = IMG_Load(imgPath.c_str());
        M_ASSERT(surf != nullptr, "Failed to load tileset PNG");

        SDL_Renderer* sdl = Renderer::Instance().GetRenderer();
        info.texture = SDL_CreateTextureFromSurface(sdl, surf);
        SDL_FreeSurface(surf);

        M_ASSERT(info.texture != nullptr, "Failed to create texture");
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

    // 1. Draw image layers (background)
    for (const auto& img : m_imageLayers)
    {
        SDL_Rect dst;
        dst.x = img.x;
        dst.y = img.y + m_yOffset;
        SDL_QueryTexture(img.texture, nullptr, nullptr, &dst.w, &dst.h);
        SDL_RenderCopy(sdl, img.texture, nullptr, &dst);
    }

    // 2. Draw tile layers
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

void TileMap::LoadCollisionObjects()
{
    m_collisionShapes.clear();

    const std::vector<std::unique_ptr<tmx::Layer>>& layers = m_map.getLayers();

    for (size_t i = 0; i < layers.size(); ++i)
    {
        if (layers[i]->getType() != tmx::Layer::Type::Object)
            continue;

        if (layers[i]->getName() != "collision")
            continue;

        const tmx::ObjectGroup& objLayer = layers[i]->getLayerAs<tmx::ObjectGroup>();
        const std::vector<tmx::Object>& objects = objLayer.getObjects();

        for (size_t o = 0; o < objects.size(); ++o)
        {
            const tmx::Object& obj = objects[o];

            CollisionShape shape;
            shape.x = obj.getPosition().x;
            shape.y = obj.getPosition().y + m_yOffset;

            if (obj.getShape() == tmx::Object::Shape::Rectangle)
            {
                shape.type = CollisionType::Rectangle;
                shape.width = obj.getAABB().width;
                shape.height = obj.getAABB().height;
            }
            else if (obj.getShape() == tmx::Object::Shape::Polygon)
            {
                shape.type = CollisionType::Polygon;

                const std::vector<tmx::Vector2f>& pts = obj.getPoints();
                for (size_t p = 0; p < pts.size(); ++p)
                {
                    shape.points.push_back(
                        Point(
                            (unsigned int)(shape.x + pts[p].x),
                            (unsigned int)(shape.y + pts[p].y)
                        )
                    );
                }
            }

            m_collisionShapes.push_back(shape);
        }
    }
}

void TileMap::LoadImageLayers()
{
    m_imageLayers.clear();

    const auto& layers = m_map.getLayers();

    for (size_t i = 0; i < layers.size(); ++i)
    {
        if (layers[i]->getType() != tmx::Layer::Type::Image)
            continue;

        const tmx::ImageLayer& imgLayer =
            layers[i]->getLayerAs<tmx::ImageLayer>();

        // Get the image file path defined in Tiled
        string imgPath = imgLayer.getImagePath();
        if (imgPath.empty())
            continue;

        SDL_Surface* surf = IMG_Load(imgPath.c_str());
        if (!surf)
            continue;

        SDL_Renderer* sdl = Renderer::Instance().GetRenderer();
        SDL_Texture* tex = SDL_CreateTextureFromSurface(sdl, surf);
        SDL_FreeSurface(surf);

        if (!tex)
            continue;

        // Get the image layer offset set in Tiled
        auto offset = imgLayer.getOffset();

        ImageLayerInfo info;
        info.texture = tex;
        info.x = (int)offset.x;
        info.y = (int)offset.y;

        m_imageLayers.push_back(info);
    }
}

bool TileMap::CheckGroundCollision(
    float px, float py,
    float pw, float ph,
    float& outGroundY
) const
{
    float playerBottom = py + ph;
    float bestY = 100000.0f;
    bool found = false;

    for (const auto& shape : m_collisionShapes)
    {
        if (shape.type == CollisionType::Rectangle)
        {
            if (px + pw <= shape.x) continue;
            if (px >= shape.x + shape.width) continue;

            if (playerBottom <= shape.y && shape.y < bestY)
            {
                bestY = shape.y;
                found = true;
            }
        }
        else if (shape.type == CollisionType::Polygon)
        {
            for (size_t i = 0; i < shape.points.size(); ++i)
            {
                const Point& a = shape.points[i];
                const Point& b = shape.points[(i + 1) % shape.points.size()];

                if (a.Y != b.Y) continue;

                float minX = (a.X < b.X) ? a.X : b.X;
                float maxX = (a.X > b.X) ? a.X : b.X;

                if (px + pw > minX && px < maxX)
                {
                    if (playerBottom <= a.Y && a.Y < bestY)
                    {
                        bestY = a.Y;
                        found = true;
                    }
                }
            }
        }
    }

    if (found)
    {
        outGroundY = bestY;
        return true;
    }

    return false;
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
