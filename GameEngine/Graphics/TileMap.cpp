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
    LoadSpawnPoint();
    LoadCoinSpawnPoints();
    LoadEnemySpawnPoints();
    LoadEnemyZones();

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
    
    // 1. Draw image layers (background)
    for (const auto& img : m_imageLayers)
    {
        int imgWidth, imgHeight;
        SDL_QueryTexture(img.texture, nullptr, nullptr, &imgWidth, &imgHeight);
        
        // Calculate how many times to repeat the background
        int startImgIndex = (int)floor((cameraX - img.x) / imgWidth);
        int endImgIndex = (int)ceil((cameraX + screenWidth - img.x) / imgWidth);
        
        for (int imgIndex = startImgIndex; imgIndex <= endImgIndex; ++imgIndex)
        {
            SDL_Rect dst;
            dst.x = img.x + (imgIndex * imgWidth) - (int)cameraX;
            dst.y = img.y;
            dst.w = imgWidth;
            dst.h = imgHeight;
            SDL_RenderCopy(sdl, img.texture, nullptr, &dst);
        }
    }
    
    // 2. Render multiple instances of the map
    for (int mapIndex = startMapIndex; mapIndex <= endMapIndex; ++mapIndex)
    {
        int mapOffsetX = mapIndex * mapPixelWidth;

        // 3. Draw tile layers
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
                    dst.y = y * m_tileHeight;
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

void TileMap::LoadSpawnPoint()
{
    m_hasSpawnPoint = false;

    const std::vector<std::unique_ptr<tmx::Layer>>& layers = m_map.getLayers();

    for (size_t i = 0; i < layers.size(); ++i)
    {
        if (layers[i]->getType() != tmx::Layer::Type::Group)
            continue;

        const tmx::LayerGroup& group = layers[i]->getLayerAs<tmx::LayerGroup>();
        const std::vector<std::unique_ptr<tmx::Layer>>& subLayers = group.getLayers();

        for (size_t j = 0; j < subLayers.size(); ++j)
        {
            if (subLayers[j]->getType() != tmx::Layer::Type::Object)
                continue;

            if (subLayers[j]->getName() != "playerSpawn")
                continue;

            const tmx::ObjectGroup& objLayer = subLayers[j]->getLayerAs<tmx::ObjectGroup>();
            const std::vector<tmx::Object>& objects = objLayer.getObjects();

            if (objects.size() > 0)
            {
                const tmx::Object& spawnObj = objects[0];
                m_spawnX = spawnObj.getPosition().x;
                m_spawnY = spawnObj.getPosition().y;
                m_hasSpawnPoint = true;
                break;
            }
        }

        if (m_hasSpawnPoint)
            break;
    }
}

bool TileMap::GetPlayerSpawnPoint(float& outX, float& outY) const
{
    if (m_hasSpawnPoint)
    {
        outX = m_spawnX;
        outY = m_spawnY;
        return true;
    }
    return false;
}

void TileMap::LoadCoinSpawnPoints()
{
    m_coinSpawnPoints.clear();

    const std::vector<std::unique_ptr<tmx::Layer>>& layers = m_map.getLayers();

    for (size_t i = 0; i < layers.size(); ++i)
    {
        if (layers[i]->getType() != tmx::Layer::Type::Group)
            continue;

        const tmx::LayerGroup& group = layers[i]->getLayerAs<tmx::LayerGroup>();
        const std::vector<std::unique_ptr<tmx::Layer>>& subLayers = group.getLayers();

        for (size_t j = 0; j < subLayers.size(); ++j)
        {
            if (subLayers[j]->getType() != tmx::Layer::Type::Object)
                continue;

            if (subLayers[j]->getName() != "coinSpawn")
                continue;

            const tmx::ObjectGroup& objLayer = subLayers[j]->getLayerAs<tmx::ObjectGroup>();
            const std::vector<tmx::Object>& objects = objLayer.getObjects();

            for (size_t k = 0; k < objects.size(); ++k)
            {
                const tmx::Object& coinObj = objects[k];
                float x = coinObj.getPosition().x;
                float y = coinObj.getPosition().y;
                m_coinSpawnPoints.push_back(std::make_pair(x, y));
            }
            break;
        }
    }
}

void TileMap::LoadEnemySpawnPoints()
{
    m_enemySpawnPoints.clear();

    const std::vector<std::unique_ptr<tmx::Layer>>& layers = m_map.getLayers();

    for (size_t i = 0; i < layers.size(); ++i)
    {
        if (layers[i]->getType() != tmx::Layer::Type::Group)
            continue;

        const tmx::LayerGroup& group = layers[i]->getLayerAs<tmx::LayerGroup>();
        const std::vector<std::unique_ptr<tmx::Layer>>& subLayers = group.getLayers();

        for (size_t j = 0; j < subLayers.size(); ++j)
        {
            if (subLayers[j]->getType() != tmx::Layer::Type::Object)
                continue;

            if (subLayers[j]->getName() != "enemySpawn")
                continue;

            const tmx::ObjectGroup& objLayer = subLayers[j]->getLayerAs<tmx::ObjectGroup>();
            const std::vector<tmx::Object>& objects = objLayer.getObjects();

            for (size_t k = 0; k < objects.size(); ++k)
            {
                const tmx::Object& enemyObj = objects[k];
                float x = enemyObj.getPosition().x;
                float y = enemyObj.getPosition().y;
                m_enemySpawnPoints.push_back(std::make_pair(x, y));
            }
            break;
        }
    }
}

void TileMap::LoadEnemyZones()
{
    m_enemyZonePoints.clear();

    const std::vector<std::unique_ptr<tmx::Layer>>& layers = m_map.getLayers();

    for (size_t i = 0; i < layers.size(); ++i)
    {
        if (layers[i]->getType() != tmx::Layer::Type::Object)
            continue;

        if (layers[i]->getName() != "enemyZones")
            continue;

        const tmx::ObjectGroup& objLayer = layers[i]->getLayerAs<tmx::ObjectGroup>();
        const std::vector<tmx::Object>& objects = objLayer.getObjects();

        for (size_t j = 0; j < objects.size(); ++j)
        {
            const tmx::Object& zoneObj = objects[j];
            float x = zoneObj.getPosition().x;
            float y = zoneObj.getPosition().y;
            m_enemyZonePoints.push_back(std::make_pair(x, y));
        }
        break;
    }
}

bool TileMap::GetEnemyZoneBoundaries(float spawnX, float spawnY, float& outLeftX, float& outRightX) const
{
    // Find the closest Enemy_Left and Enemy_Right points to this spawn point
    float closestLeftX = spawnX - 100.0f; // Default fallback
    float closestRightX = spawnX + 100.0f; // Default fallback
    float closestLeftDist = 999999.0f;
    float closestRightDist = 999999.0f;
    
    // Search through all zone points
    for (const auto& point : m_enemyZonePoints)
    {
        float zoneX = point.first;
        float zoneY = point.second;
        
        // Calculate distance from spawn point
        float dx = zoneX - spawnX;
        float dy = zoneY - spawnY;
        float dist = sqrt(dx * dx + dy * dy);
        
        // If this point is to the left of spawn
        if (zoneX < spawnX && dist < closestLeftDist)
        {
            closestLeftX = zoneX;
            closestLeftDist = dist;
        }
        // If this point is to the right of spawn
        else if (zoneX > spawnX && dist < closestRightDist)
        {
            closestRightX = zoneX;
            closestRightDist = dist;
        }
    }
    
    outLeftX = closestLeftX;
    outRightX = closestRightX;
    
    return true;
}

// Check collision from top (player standing on box)
bool TileMap::CheckCollisionTop(float px, float py, float pw, float ph, float& outGroundY) const
{
    float playerBottom = py + ph;
    float bestY = 100000.0f;
    bool found = false;
    
    int mapPixelWidth = GetMapPixelWidth();
    int startMapIndex = (int)floor((px - pw) / mapPixelWidth);
    int endMapIndex = (int)ceil((px + pw * 2) / mapPixelWidth);

    for (int mapIndex = startMapIndex; mapIndex <= endMapIndex; ++mapIndex)
    {
        float mapOffsetX = mapIndex * mapPixelWidth;
        
        for (const auto& shape : m_collisionShapes)
        {
            if (shape.type == CollisionType::Rectangle)
            {
                float shapeX = shape.x + mapOffsetX;
                float shapeY = shape.y;
                float shapeRight = shapeX + shape.width;
                float shapeBottom = shapeY + shape.height;

                // Check horizontal overlap
                if (px + pw <= shapeX || px >= shapeRight) continue;

                // Only detect platforms that the player is close to landing on
                if (shapeY >= py && shapeY < bestY)
                {
                    bestY = shapeY;
                    found = true;
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

// Check collision from bottom (player hitting head on box)
bool TileMap::CheckCollisionBottom(float px, float py, float pw, float ph, float& outCeilingY) const
{
    float playerTop = py;
    float bestY = -100000.0f;
    bool found = false;
    
    int mapPixelWidth = GetMapPixelWidth();
    int startMapIndex = (int)floor((px - pw) / mapPixelWidth);
    int endMapIndex = (int)ceil((px + pw * 2) / mapPixelWidth);

    for (int mapIndex = startMapIndex; mapIndex <= endMapIndex; ++mapIndex)
    {
        float mapOffsetX = mapIndex * mapPixelWidth;
        
        for (const auto& shape : m_collisionShapes)
        {
            if (shape.type == CollisionType::Rectangle)
            {
                float shapeX = shape.x + mapOffsetX;
                float shapeY = shape.y;
                float shapeRight = shapeX + shape.width;
                float shapeBottom = shapeY + shape.height;

                // Check horizontal overlap
                if (px + pw <= shapeX || px >= shapeRight) continue;

                // Check if player is jumping into the bottom of the box
                // Only check boxes that are above the player
                if (playerTop <= shapeBottom && playerTop >= shapeBottom - 5.0f)
                {
                    // Find the closest ceiling above the player
                    if (shapeBottom <= py + ph && shapeBottom > bestY)
                    {
                        bestY = shapeBottom;
                        found = true;
                    }
                }
            }
        }
    }

    if (found)
    {
        outCeilingY = bestY;
        return true;
    }
    return false;
}

// Check collision from left (player hitting box from left side)
bool TileMap::CheckCollisionLeft(float px, float py, float pw, float ph, float& outWallX) const
{
    float playerRight = px + pw;
    float bestX = 100000.0f;
    bool found = false;
    
    int mapPixelWidth = GetMapPixelWidth();
    int startMapIndex = (int)floor((px - pw) / mapPixelWidth);
    int endMapIndex = (int)ceil((px + pw * 2) / mapPixelWidth);

    for (int mapIndex = startMapIndex; mapIndex <= endMapIndex; ++mapIndex)
    {
        float mapOffsetX = mapIndex * mapPixelWidth;
        
        for (const auto& shape : m_collisionShapes)
        {
            if (shape.type == CollisionType::Rectangle)
            {
                float shapeX = shape.x + mapOffsetX;
                float shapeY = shape.y;
                float shapeRight = shapeX + shape.width;
                float shapeBottom = shapeY + shape.height;

                // Check vertical overlap
                if (py + ph <= shapeY || py >= shapeBottom) continue;

                // Check if player is moving into the left side of the box
                if (playerRight >= shapeX && playerRight <= shapeX + 5.0f && shapeX < bestX)
                {
                    bestX = shapeX;
                    found = true;
                }
            }
        }
    }

    if (found)
    {
        outWallX = bestX;
        return true;
    }
    return false;
}

// Check collision from right (player hitting box from right side)
bool TileMap::CheckCollisionRight(float px, float py, float pw, float ph, float& outWallX) const
{
    float playerLeft = px;
    float bestX = -100000.0f;
    bool found = false;
    
    int mapPixelWidth = GetMapPixelWidth();
    int startMapIndex = (int)floor((px - pw) / mapPixelWidth);
    int endMapIndex = (int)ceil((px + pw * 2) / mapPixelWidth);

    for (int mapIndex = startMapIndex; mapIndex <= endMapIndex; ++mapIndex)
    {
        float mapOffsetX = mapIndex * mapPixelWidth;
        
        for (const auto& shape : m_collisionShapes)
        {
            if (shape.type == CollisionType::Rectangle)
            {
                float shapeX = shape.x + mapOffsetX;
                float shapeY = shape.y;
                float shapeRight = shapeX + shape.width;
                float shapeBottom = shapeY + shape.height;

                // Check vertical overlap
                if (py + ph <= shapeY || py >= shapeBottom) continue;

                // Check if player is moving into the right side of the box
                if (playerLeft <= shapeRight && playerLeft >= shapeRight - 5.0f && shapeRight > bestX)
                {
                    bestX = shapeRight;
                    found = true;
                }
            }
        }
    }

    if (found)
    {
        outWallX = bestX;
        return true;
    }
    return false;
}

void TileMap::RenderCollisionBoxes(Renderer* _renderer, Camera* _camera)
{
    SDL_Renderer* sdl = _renderer->GetRenderer();
    int mapPixelWidth = GetMapPixelWidth();
    
    Point screenSize = _renderer->GetWindowSize();
    int screenWidth = screenSize.X;
    
    float cameraX = _camera ? _camera->GetX() : 0.0f;
    
    int startMapIndex = (int)floor(cameraX / mapPixelWidth);
    int endMapIndex = (int)ceil((cameraX + screenWidth) / mapPixelWidth);
    
    SDL_SetRenderDrawColor(sdl, 255, 0, 0, 255);
    
    // Render collision boxes for each map instance
    for (int mapIndex = startMapIndex; mapIndex <= endMapIndex; ++mapIndex)
    {
        float mapOffsetX = mapIndex * mapPixelWidth;
        
        for (const auto& shape : m_collisionShapes)
        {
            if (shape.type == CollisionType::Rectangle)
            {
                // Apply offset and camera transform
                float shapeX = shape.x + mapOffsetX;
                float screenX = _camera ? _camera->WorldToScreenX(shapeX) : shapeX;
                
                SDL_Rect rect;
                rect.x = (int)screenX;
                rect.y = (int)shape.y;
                rect.w = (int)shape.width;
                rect.h = (int)shape.height;
                
                SDL_RenderDrawRect(sdl, &rect);
            }
            else if (shape.type == CollisionType::Polygon)
            {
                // Draw polygon edges
                for (size_t i = 0; i < shape.points.size(); ++i)
                {
                    const Point& a = shape.points[i];
                    const Point& b = shape.points[(i + 1) % shape.points.size()];
                    
                    // Apply offset and camera transform
                    float aX = a.X + mapOffsetX;
                    float bX = b.X + mapOffsetX;
                    float screenAX = _camera ? _camera->WorldToScreenX(aX) : aX;
                    float screenBX = _camera ? _camera->WorldToScreenX(bX) : bX;
                    
                    SDL_RenderDrawLine(sdl, 
                        (int)screenAX, (int)a.Y,
                        (int)screenBX, (int)b.Y);
                }
            }
        }
    }
}
