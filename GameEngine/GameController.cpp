#include "GameController.h"
#include "Renderer.h"
#include "InputController.h"
#include "Player.h"
#include "SpriteAnim.h"
#include "SpriteSheet.h"
#include "AssetController.h"
#include "Keyboard.h"
#include "Timing.h"

GameController::GameController()
{
    m_sdlEvent = { };
    m_renderer = nullptr;
    m_input = nullptr;
    m_player = nullptr;
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

    // Systems
    m_renderer = &Renderer::Instance();
    m_renderer->Initialize();
    m_input = &InputController::Instance();

    // Player
    m_player = new Player();
    m_player->Initialize();
}

void GameController::ShutDown()
{
    delete m_player;
    m_player = nullptr;

    delete SpriteAnim::Pool;
    SpriteAnim::Pool = nullptr;

    delete SpriteSheet::Pool;
    SpriteSheet::Pool = nullptr;
}

void GameController::HandleInput(SDL_Event _event)
{
    if (_event.type == SDL_QUIT)
        m_quit = true;
    if ((m_sdlEvent.type == SDL_QUIT) || (m_input->KB()->KeyUp(m_sdlEvent, SDLK_ESCAPE))) 
        m_quit = true; 
}

void GameController::RunGame()
{
    Initialize();

    while (!m_quit)
    {
        Timing::Instance().Tick();
        float deltaTime = Timing::Instance().GetDeltaTime();

        m_renderer->SetDrawColor(Color(255, 255, 255, 255));
        m_renderer->ClearScreen();

        while (SDL_PollEvent(&m_sdlEvent) != 0)
            HandleInput(m_sdlEvent);

        m_player->Update(deltaTime);
        m_player->Render(m_renderer);

        SDL_RenderPresent(m_renderer->GetRenderer());
    }
}

