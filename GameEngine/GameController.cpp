#include "GameController.h"
#include "Renderer.h"
#include "SpriteAnim.h"
#include "SpriteSheet.h"
#include "TTFont.h"
#include "Timing.h"
#include "RenderTarget.h"
#include "InputController.h"
#include "Keyboard.h"

GameController::GameController()
{
    m_sdlEvent = { };
    m_renderer = nullptr;
    m_fArial20 = nullptr;
    m_input = nullptr;
    m_quit = false;
}

GameController::~GameController()
{
    ShutDown();
}

void GameController::Initialize()
{
    m_renderer = &Renderer::Instance();
    m_renderer->Initialize();
    m_input = &InputController::Instance();
    m_fArial20 = new TTFont();
    m_fArial20->Initialize(20);
}

void GameController::ShutDown()
{
    delete m_fArial20;
}

void GameController::HandleInput(SDL_Event _event)
{
    string temp;
    if ((m_sdlEvent.type == SDL_QUIT) ||
        (m_input->KB()->KeyUp(m_sdlEvent, SDLK_ESCAPE)))
    {
        m_quit = true;
    }
    else if ((temp = m_input->KB()->TextInput(_event)) != "")
    {
        m_text += temp;
    }
    else if (m_input->KB()->KeyUp(m_sdlEvent, SDLK_RETURN))
    {
        m_text = "";
    }
}

void GameController::RunGame()
{
    Initialize();
    
    while (!m_quit)
    {
        m_renderer->SetDrawColor(Color(255, 255, 255, 255));
        m_renderer->ClearScreen();

        while (SDL_PollEvent(&m_sdlEvent) != 0)
        {
            HandleInput(m_sdlEvent);
        }

        m_fArial20->Write(m_renderer->GetRenderer(), m_text.c_str(), SDL_Color{ 0,255,0 }, SDL_Point{ 250,200 });
        SDL_RenderPresent(m_renderer->GetRenderer());
    }
}

