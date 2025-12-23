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
        info.firstGID = ts.getFirstGID();
        info.tileWidth = ts.getTileSize().x;
        info.tileHeight = ts.getTileSize().y;
        info.columns = ts.getColumnCount();
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

void TileMap::Render(Renderer* _renderer, Camera* _camera)
{
    SDL_Renderer* sdl = _renderer->GetRenderer();
    int mapPixelWidth = GetMapPixelWidth();
    
    // Get screen width to determine how many map copies to render
    Point screenSize = _renderer->GetWindowSize();
    int screenWidth = screenSize.X;
    
    float cameraX = _camera ? _camera->GetX() : 0.0f;
    
    // Calculate which map instances we need to render
    int startMapIndex = (int)floor(cameraX / mapPixelWidth);
    int endMapIndex = (int)ceil((cameraX + screenWidth) / mapPixelWidth);
    
    // Render multiple instances of the map for endless scrolling
    for (int mapIndex = startMapIndex; mapIndex <= endMapIndex; ++mapIndex)
    {
        int mapOffsetX = mapIndex * mapPixelWidth;
        
        // 1. Draw image layers (background)
        for (const auto& img : m_imageLayers)
        {
            SDL_Rect dst;
            dst.x = img.x + mapOffsetX - (int)cameraX;
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
                    dst.x = x * m_tileWidth + mapOffsetX - (int)cameraX;
                    dst.y = y * m_tileHeight + m_yOffset;
                    dst.w = m_tileWidth;
                    dst.h = m_tileHeight;

                    SDL_RenderCopy(sdl, ts->texture, &src, &dst);
                }
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
            shape.y = obj.getPosition().y;

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
    
    int mapPixelWidth = GetMapPixelWidth();
    
    // Determine which map instances the player might be colliding with
    int startMapIndex = (int)floor((px - pw) / mapPixelWidth);
    int endMapIndex = (int)ceil((px + pw * 2) / mapPixelWidth);

    for (int mapIndex = startMapIndex; mapIndex <= endMapIndex; ++mapIndex)
    {
        float mapOffsetX = mapIndex * mapPixelWidth;
        
        for (const auto& shape : m_collisionShapes)
        {
            if (shape.type == CollisionType::Rectangle)
            {
                // Apply offset to collision shape (including map repetition)
                float shapeX = shape.x + mapOffsetX;
                float shapeY = shape.y + m_yOffset;

                // Check horizontal overlap
                if (px + pw <= shapeX) continue;
                if (px >= shapeX + shape.width) continue;

                // Rectangle's top edge is the ground surface
                if (playerBottom <= shapeY + 2.0f && shapeY < bestY)
                {
                    bestY = shapeY;
                    found = true;
                }
            }
            else if (shape.type == CollisionType::Polygon)
            {
                // Check each edge of the polygon
                for (size_t i = 0; i < shape.points.size(); ++i)
                {
                    const Point& a = shape.points[i];
                    const Point& b = shape.points[(i + 1) % shape.points.size()];

                    // Apply offset to polygon points (including map repetition)
                    float aX = a.X + mapOffsetX;
                    float bX = b.X + mapOffsetX;
                    float aY = a.Y + m_yOffset;
                    float bY = b.Y + m_yOffset;

                    // Check if edge is horizontal (or nearly horizontal)
                    if (abs((int)aY - (int)bY) > 2) continue;

                    float minX = (aX < bX) ? aX : bX;
                    float maxX = (aX > bX) ? aX : bX;

                    // Check horizontal overlap
                    if (px + pw > minX && px < maxX)
                    {
                        if (playerBottom <= aY + 5.0f && aY < bestY)
                        {
                            bestY = aY;
                            found = true;
                        }
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