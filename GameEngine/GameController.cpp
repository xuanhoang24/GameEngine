#include "GameController.h"
#include "Renderer.h"
#include "TTFont.h"
#include "InputController.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Controller.h"

GameController::GameController()
{
    m_sdlEvent = { };
    m_renderer = nullptr;
    m_fArial20 = nullptr;
    m_input = nullptr;
    m_quit = false;
    m_smPos = "";
    m_mPos = { };
    m_ctInfo = "";
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
    else if (m_input->MS()->Moved(m_sdlEvent, m_mPos))
    {
        m_smPos = "Mouse Position [" + to_string(m_mPos.X) + ";" + to_string(m_mPos.Y) + "]";
    }
    else if ((m_input->CT()->Added(m_sdlEvent)) ||
        (m_input->CT()->Removed(m_sdlEvent)) ||
        (m_input->CT()->ProcessButtons(m_sdlEvent)))
    {
        m_ctInfo = m_input->CT()->ToString();
    }
        
    m_input->MS()->ProcessButtons(_event);
}

void GameController::RunGame()
{
    Initialize();
    m_input->CT()->DetectControllers();

    while (!m_quit)
    {
        m_renderer->SetDrawColor(Color(255, 255, 255, 255));
        m_renderer->ClearScreen();

        while (SDL_PollEvent(&m_sdlEvent) != 0)
        {
            HandleInput(m_sdlEvent);
        }

        m_fArial20->Write(m_renderer->GetRenderer(), m_text.c_str(), SDL_Color{ 0,255,0 }, SDL_Point{ 250,200 });
        m_fArial20->Write(m_renderer->GetRenderer(), m_smPos.c_str(), SDL_Color{ 0,0,255 }, SDL_Point{ 250,200 });
        m_fArial20->Write(m_renderer->GetRenderer(), ("Left: " + to_string(m_input->MS()->GetButLDown())).c_str(), SDL_Color{ 0,0,255 }, SDL_Point{ 250,240 });
        m_fArial20->Write(m_renderer->GetRenderer(), ("Middle: " + to_string(m_input->MS()->GetButMDown())).c_str(), SDL_Color{ 0,0,255 }, SDL_Point{ 250,260 });
        m_fArial20->Write(m_renderer->GetRenderer(), ("Right: " + to_string(m_input->MS()->GetButRDown())).c_str(), SDL_Color{ 0,0,255 }, SDL_Point{ 250,280 });
        m_fArial20->Write(m_renderer->GetRenderer(), m_ctInfo.c_str(), SDL_Color{ 255,0,0 }, SDL_Point{ 250,300 });

        SDL_RenderPresent(m_renderer->GetRenderer());
    }
}

