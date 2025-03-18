#include "GameController.h"
#include "Renderer.h"
#include "TTFont.h"
#include "InputController.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Controller.h"
#include "AudioController.h"
#include "SoundEffect.h"
#include "Song.h"

GameController::GameController()
{
    m_sdlEvent = { };
    m_renderer = nullptr;
    m_fArial20 = nullptr;
    m_input = nullptr;
    m_quit = false;
    m_audio = nullptr;
    m_effect = nullptr;
    m_song = nullptr;
}

GameController::~GameController()
{
    ShutDown();
}

void GameController::Initialize()
{
    AssetController::Instance().Initialize(10000000); // Alocate 10MB
    m_renderer = &Renderer::Instance();
    m_renderer->Initialize();
    m_input = &InputController::Instance();
    m_fArial20 = new TTFont();
    m_fArial20->Initialize(20);
    m_audio = &AudioController::Instance();
    m_effect = m_audio->LoadEffect("../Assets/Audio/Effects/Whoosh.wav");
    m_song = m_audio->LoadSong("../Assets/Audio/Music/Track1.mp3");
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
    else if (m_input->KB()->KeyUp(_event, SDLK_p))
    {
        m_audio->Play(m_effect);
    }
    else if (m_input->KB()->KeyUp(_event, SDLK_a))
    {
        m_audio->Play(m_song);
    }
    else if (m_input->KB()->KeyUp(_event, SDLK_s))
    {
        m_audio->PauseMusic();
    }
    else if (m_input->KB()->KeyUp(_event, SDLK_d))
    {
        m_audio->ResumeMusic();
    }
    else if (m_input->KB()->KeyUp(_event, SDLK_f))
    {
        m_audio->StopMusic();
    }
        
    m_input->MS()->ProcessButtons(_event);
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

        string song = "Current Song: " + m_audio->GetMusicTitle();
        if (m_audio->GetMusicLength() != "")
        {
            song += " " + to_string((int)m_audio->MusicPosition()) + "/" + m_audio->GetMusicLength();
        }
        m_fArial20->Write(m_renderer->GetRenderer(), song.c_str(), { 0,0,255 }, { 10,10 });
        m_fArial20->Write(m_renderer->GetRenderer(), ("Current Effect: " + m_audio->GetCurrentEffect()).c_str(), { 0,0,255 }, { 10,30 });
        SDL_RenderPresent(m_renderer->GetRenderer());
    }
}

