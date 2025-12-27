#include "../Core/GameController.h"
#include "../Graphics/Renderer.h"
#include "../Graphics/Camera.h"
#include "../Graphics/TTFont.h"
#include "../Input/InputController.h"
#include "../Game/Player.h"
#include "../Game/Coin.h"
#include "../Game/Enemy.h"
#include "../Graphics/SpriteAnim.h"
#include "../Graphics/SpriteSheet.h"
#include "../Graphics/Texture.h"
#include "../Resources/AssetController.h"
#include "../Input/Keyboard.h"
#include "../Core/Timing.h"
#include "../Game/GameMap.h"
#include <sstream>

GameController::GameController()
{
    m_sdlEvent = { };
    m_renderer = nullptr;
    m_input = nullptr;
    m_player = nullptr;
    m_camera = nullptr;
    m_font = nullptr;
    m_quit = false;
    m_score = 0;
    m_coins.clear();
    m_enemies.clear();
}

GameController::~GameController()
{
    ShutDown();
}

void GameController::Initialize()
{
    // Allocate 50MB
    AssetController::Instance().Initialize(50000000);
    SpriteAnim::Pool = new ObjectPool<SpriteAnim>();
    SpriteSheet::Pool = new ObjectPool<SpriteSheet>();
    Texture::Pool = new ObjectPool<Texture>();

    // Systems
    m_renderer = &Renderer::Instance();
    m_renderer->Initialize();
    m_input = &InputController::Instance();

    // Camera
    m_camera = new Camera();

    // Player
    m_player = new Player();
    m_player->Initialize();

    // Game Map
    g_Map = new GameMap();
    g_Map->Load("../Assets/Maps/Map1.tmx");
    
    // Connect player to map for collision
    m_player->SetGameMap(g_Map);
    
    // Set player spawn position from map
    float spawnX, spawnY;
    if (g_Map->GetPlayerSpawnPoint(spawnX, spawnY))
    {
        m_player->SetSpawnPosition(spawnX, spawnY);
    }
    
    // Spawn coins from map
    m_coins = Coin::SpawnCoinsFromMap(g_Map);
    
    // Spawn enemies from map
    m_enemies = Enemy::SpawnEnemiesFromMap(g_Map);
    
    // Initialize font for score display
    m_font = new TTFont();
    m_font->Initialize(24);
    m_score = 0;
}

void GameController::ShutDown()
{
    delete m_font;
    m_font = nullptr;
    
    delete m_player;
    m_player = nullptr;

    delete m_camera;
    m_camera = nullptr;
    
    // Clean up coins
    for (Coin* coin : m_coins)
    {
        delete coin;
    }
    m_coins.clear();
    
    // Clean up enemies
    for (Enemy* enemy : m_enemies)
    {
        delete enemy;
    }
    m_enemies.clear();

    delete SpriteAnim::Pool;
    SpriteAnim::Pool = nullptr;

    delete SpriteSheet::Pool;
    SpriteSheet::Pool = nullptr;

    delete Texture::Pool;
    Texture::Pool = nullptr;
}

void GameController::HandleInput(SDL_Event _event)
{
    if (_event.type == SDL_QUIT)
        m_quit = true;
    if ((m_sdlEvent.type == SDL_QUIT) || (m_input->KB()->KeyUp(m_sdlEvent, SDLK_ESCAPE)))
        m_quit = true;

    m_player->HandleInput(_event);
}

void GameController::RunGame()
{
    Timing* t = &Timing::Instance();
    t->SetFPS(60);

    Initialize();

    while (!m_quit)
    {
        t->Tick();
        m_renderer->SetDrawColor(Color(255, 255, 255, 255));
        m_renderer->ClearScreen();

        while (SDL_PollEvent(&m_sdlEvent) != 0)
            HandleInput(m_sdlEvent);

        m_player->Update(t->GetDeltaTime());
        
        // Update camera to follow player (before constraining player)
        m_camera->FollowPlayer(m_player, m_renderer);
        
        // Constrain player to camera view (prevent going left off-screen)
        float cameraLeftEdge = m_camera->GetMaxX();
        float playerWorldX = m_player->GetWorldX();
        
        // If player tries to go left of the camera's left edge, clamp position
        if (playerWorldX < cameraLeftEdge)
        {
            // Set player position to camera left edge
            m_player->SetSpawnPosition(cameraLeftEdge + m_player->GetWidth() * 0.5f, m_player->GetWorldY() + m_player->GetHeight());
        }
        
        // Get camera position for respawn checks
        float cameraX = m_camera->GetX();
        int mapPixelWidth = 1600; // 100 tiles * 16 pixels
        Point screenSize = m_renderer->GetWindowSize();
        int screenWidth = screenSize.X;
        
        // Update coins
        for (Coin* coin : m_coins)
        {
            coin->Update(t->GetDeltaTime(), cameraX, screenWidth, mapPixelWidth);
        }
        
        // Update enemies
        for (Enemy* enemy : m_enemies)
        {
            enemy->Update(t->GetDeltaTime(), cameraX, screenWidth, mapPixelWidth);
        }
        
        // Check collisions
        CheckPlayerCoinCollisions();
        CheckPlayerEnemyCollisions();
        
        g_Map->Render(m_renderer, m_camera);
        
        // Render enemies
        for (Enemy* enemy : m_enemies)
        {
            enemy->Render(m_renderer, m_camera);
        }
        
        // Render coins
        for (Coin* coin : m_coins)
        {
            coin->Render(m_renderer, m_camera);
        }
        
        m_player->Render(m_renderer, m_camera);
        m_player->RenderCollisionBox(m_renderer, m_camera);
        g_Map->RenderCollisionBoxes(m_renderer, m_camera);
        
        // Render score UI
        std::stringstream ss;
        ss << "Score: " << m_score;
        SDL_Color white = { 255, 255, 255, 255 };
        SDL_Point scorePos = { 10, 10 };
        m_font->Write(m_renderer->GetRenderer(), ss.str().c_str(), white, scorePos);

        t->CapFPS();
        SDL_RenderPresent(m_renderer->GetRenderer());
    }
}

bool GameController::CheckAABBCollision(float _x1, float _y1, float _w1, float _h1,
                                        float _x2, float _y2, float _w2, float _h2)
{
    return (_x1 < _x2 + _w2 &&
            _x1 + _w1 > _x2 &&
            _y1 < _y2 + _h2 &&
            _y1 + _h1 > _y2);
}

void GameController::CheckPlayerCoinCollisions()
{
    if (m_player->IsDead())
        return;
    
    float playerX, playerY, playerW, playerH;
    m_player->GetCollisionBox(playerX, playerY, playerW, playerH);
    
    for (Coin* coin : m_coins)
    {
        if (!coin->IsActive())
            continue;
        
        float coinX, coinY, coinW, coinH;
        coin->GetCollisionBox(coinX, coinY, coinW, coinH);
        
        // Collision check
        if (CheckAABBCollision(playerX, playerY, playerW, playerH,
                              coinX, coinY, coinW, coinH))
        {
            m_score += coin->GetPointValue();
            coin->Collect();
        }
    }
}

void GameController::CheckPlayerEnemyCollisions()
{
    if (m_player->IsDead())
        return;
    
    float playerX, playerY, playerW, playerH;
    m_player->GetCollisionBox(playerX, playerY, playerW, playerH);
    
    for (Enemy* enemy : m_enemies)
    {
        if (!enemy->IsActive())
            continue;
        
        float enemyX, enemyY, enemyW, enemyH;
        enemy->GetCollisionBox(enemyX, enemyY, enemyW, enemyH);
        
        // Collision check
        if (CheckAABBCollision(playerX, playerY, playerW, playerH,
                              enemyX, enemyY, enemyW, enemyH))
        {
            // Check if player is jumping on top of enemy
            float playerBottom = playerY + playerH;
            float enemyTop = enemyY;
            float verticalOverlap = playerBottom - enemyTop;
            
            // If player is falling and hits enemy from above (small vertical overlap)
            if (m_player->IsMovingDown() && verticalOverlap < playerH * 0.5f)
            {
                // Kill the enemy
                enemy->Destroy();
            }
            else
            {
                // Player hit enemy from side or below - player dies
                m_player->Die();
            }
            
            break;
        }
    }
}
