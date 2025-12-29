#include "../Core/GameController.h"
#include "../Graphics/Renderer.h"
#include "../Graphics/Camera.h"
#include "../Graphics/TTFont.h"
#include "../Input/InputController.h"
#include "../Game/Player.h"
#include "../Game/GameUI.h"
#include "../Graphics/SpriteAnim.h"
#include "../Graphics/SpriteSheet.h"
#include "../Graphics/Texture.h"
#include "../Resources/AssetController.h"
#include "../Input/Keyboard.h"
#include "../Core/Timing.h"
#include "../Game/ChunkMap.h"
#include "../Game/Coin.h"
#include "../Game/Enemy.h"

GameController::GameController()
{
    m_sdlEvent = { };
    m_renderer = nullptr;
    m_input = nullptr;
    m_player = nullptr;
    m_camera = nullptr;
    m_gameUI = nullptr;
    m_chunkMap = nullptr;
    m_quit = false;
    m_score = 0;
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

    // Chunk Map - load chunk-based infinite map
    m_chunkMap = new ChunkMap();
    m_chunkMap->LoadDefaultChunks();
    
    // Set logical size based on map height (zooms camera to fit map)
    int mapHeight = m_chunkMap->GetMapPixelHeight();
    m_renderer->SetLogicalSizeFromMapHeight(mapHeight);
    
    // Connect player to chunk map for collision
    m_player->SetChunkMap(m_chunkMap);
    
    // Set initial player spawn position from map
    float spawnX, spawnY;
    m_chunkMap->GetPlayerSpawnPoint(spawnX, spawnY);
    m_player->SetSpawnPosition(spawnX, spawnY);
   
    
    // Initialize UI
    m_gameUI = new GameUI();
    m_gameUI->Initialize();
    m_score = 0;
}

void GameController::ShutDown()
{
    delete m_gameUI;
    m_gameUI = nullptr;
    
    delete m_player;
    m_player = nullptr;

    delete m_camera;
    m_camera = nullptr;
    
    delete m_chunkMap;
    m_chunkMap = nullptr;

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
    
    m_gameUI->HandleInput(_event, m_renderer);
    
    // Handle UI requests
    if (m_gameUI->IsStartRequested())
    {
        m_gameUI->SetState(UIState::Playing);
    }
    if (m_gameUI->IsRestartRequested())
    {
        RestartGame();
    }
    if (m_gameUI->IsResumeRequested())
    {
        m_gameUI->SetState(UIState::Playing);
    }
    if (m_gameUI->IsMainMenuRequested())
    {
        RestartGame();
        m_gameUI->SetState(UIState::StartScreen);
    }
    if (m_gameUI->IsExitRequested())
    {
        m_quit = true;
    }
    
    // Only handle player input when playing
    if (m_gameUI->GetState() == UIState::Playing)
    {
        m_player->HandleInput(_event);
    }
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

        // Handle Start Screen state
        if (m_gameUI->GetState() == UIState::StartScreen)
        {
            m_gameUI->Render(m_renderer, m_score);
            t->CapFPS();
            SDL_RenderPresent(m_renderer->GetRenderer());
            continue;
        }
        
        // Handle Paused state - render but don't update
        if (m_gameUI->GetState() == UIState::Paused)
        {
            m_chunkMap->Render(m_renderer, m_camera);
            RenderEntities();
            m_player->Render(m_renderer, m_camera);
            m_gameUI->Render(m_renderer, m_score);
            t->CapFPS();
            SDL_RenderPresent(m_renderer->GetRenderer());
            continue;
        }

        // Update game state to GameOver when player dies
        if (m_player->IsFullyDead() && m_gameUI->GetState() == UIState::Playing)
        {
            m_gameUI->SetState(UIState::GameOver);
        }

        m_player->Update(t->GetDeltaTime());
        
        // Update camera to follow player
        m_camera->FollowPlayer(m_player, m_renderer);
        
        // Update chunk map - spawn new chunks as needed
        Point logicalSize = m_renderer->GetLogicalSize();
        m_chunkMap->Update(m_camera->GetX(), (float)logicalSize.X);
        
        // Constrain player to camera view (prevent going left off-screen)
        float cameraLeftEdge = m_camera->GetMaxX();
        float playerWorldX = m_player->GetWorldX();
        
        if (playerWorldX < cameraLeftEdge)
        {
            m_player->SetSpawnPosition(cameraLeftEdge + m_player->GetWidth() * 0.5f, m_player->GetWorldY() + m_player->GetHeight());
        }
        
        // Render
        m_chunkMap->Render(m_renderer, m_camera);
        
        // Update collisions and render entities
        UpdateCollisions();
        RenderEntities();
        
        m_player->Render(m_renderer, m_camera);
        m_player->RenderCollisionBox(m_renderer, m_camera);
        m_chunkMap->RenderCollisionBoxes(m_renderer, m_camera);
        
        // Render UI
        m_gameUI->Render(m_renderer, m_score);

        t->CapFPS();
        SDL_RenderPresent(m_renderer->GetRenderer());
    }
}

void GameController::UpdateCollisions()
{
    float playerX, playerY, playerW, playerH;
    m_player->GetCollisionBox(playerX, playerY, playerW, playerH);
    
    // Check coin collisions
    vector<Coin*> coins = m_chunkMap->GetAllCoins();
    for (auto* coin : coins)
    {
        if (!coin || !coin->IsActive())
            continue;
        
        float coinX, coinY, coinW, coinH;
        coin->GetCollisionBox(coinX, coinY, coinW, coinH);
        
        if (playerX < coinX + coinW && playerX + playerW > coinX &&
            playerY < coinY + coinH && playerY + playerH > coinY)
        {
            coin->Collect();
            m_score += coin->GetPointValue();
        }
    }
    
    // Check enemy collisions
    vector<Enemy*> enemies = m_chunkMap->GetAllEnemies();
    for (auto* enemy : enemies)
    {
        if (!enemy || !enemy->IsActive())
            continue;
        
        float enemyX, enemyY, enemyW, enemyH;
        enemy->GetCollisionBox(enemyX, enemyY, enemyW, enemyH);
        
        if (playerX < enemyX + enemyW && playerX + playerW > enemyX &&
            playerY < enemyY + enemyH && playerY + playerH > enemyY)
        {
            float playerBottom = playerY + playerH;
            if (playerBottom <= enemyY + 8.0f && m_player->IsFalling())
            {
                enemy->Destroy();
                m_score += 50;
                m_player->Bounce();
            }
            else
            {
                m_player->TakeDamage();
            }
        }
    }
}

void GameController::RenderEntities()
{
    // Render coins
    vector<Coin*> coins = m_chunkMap->GetAllCoins();
    for (auto* coin : coins)
    {
        if (coin && coin->IsActive())
            coin->Render(m_renderer, m_camera);
    }
    
    // Render enemies
    vector<Enemy*> enemies = m_chunkMap->GetAllEnemies();
    for (auto* enemy : enemies)
    {
        if (enemy && enemy->IsActive())
            enemy->Render(m_renderer, m_camera);
    }
}

void GameController::RestartGame()
{
    m_player->Reset();
    
    // Reset chunk map
    m_chunkMap->Reset();
    
    float spawnX, spawnY;
    m_chunkMap->GetPlayerSpawnPoint(spawnX, spawnY);
    m_player->SetSpawnPosition(spawnX, spawnY);
    
    m_camera->Reset();
    m_score = 0;
    
    m_gameUI->SetState(UIState::Playing);
}
