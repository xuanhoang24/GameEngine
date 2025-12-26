#include "../Core/GameController.h"
#include "../Graphics/Renderer.h"
#include "../Graphics/Camera.h"
#include "../Input/InputController.h"
#include "../Game/Player.h"
#include "../Graphics/SpriteAnim.h"
#include "../Graphics/SpriteSheet.h"
#include "../Graphics/Texture.h"
#include "../Resources/AssetController.h"
#include "../Input/Keyboard.h"
#include "../Core/Timing.h"
#include "../Game/GameMap.h"

GameController::GameController()
{
    m_sdlEvent = { };
    m_renderer = nullptr;
    m_input = nullptr;
    m_player = nullptr;
    m_camera = nullptr;
    m_quit = false;
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
    g_Map->Load("../Assets/Maps/untitled.tmx");
    
    // Connect player to map for collision
    m_player->SetGameMap(g_Map);
    
    // Set player spawn position from map
    float spawnX, spawnY;
    if (g_Map->GetPlayerSpawnPoint(spawnX, spawnY))
    {
        m_player->SetSpawnPosition(spawnX, spawnY);
    }
}

void GameController::ShutDown()
{
    delete m_player;
    m_player = nullptr;

    delete m_camera;
    m_camera = nullptr;

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
        
        // Update camera to follow player
        m_camera->FollowPlayer(m_player, m_renderer);
        
        g_Map->Render(m_renderer, m_camera);
        m_player->Render(m_renderer, m_camera);
        m_player->RenderCollisionBox(m_renderer, m_camera);
        g_Map->RenderCollisionBoxes(m_renderer, m_camera);

        t->CapFPS();
        SDL_RenderPresent(m_renderer->GetRenderer());
    }
}
