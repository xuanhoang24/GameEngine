#include "GameController.h"
#include "Renderer.h"
#include "SpriteAnim.h"
#include "SpriteSheet.h"
#include "TTFont.h"
#include "Timing.h"
#include <sstream> 

GameController::GameController()
{
    m_sdlEvent = { };
}

GameController::~GameController()
{
}

void GameController::RunGame()
{
    AssetController::Instance().Initialize(10000000); // Allocate 10MB
    Renderer* r = &Renderer::Instance();
    Timing* t = &Timing::Instance();
    r->Initialize();
    r->EnumerateDisplayModes();

    TTFont* font = new TTFont();
    font->Initialize(20);

    while (m_sdlEvent.type != SDL_QUIT)
    {
        t->Tick();
        
        SDL_PollEvent(&m_sdlEvent);
        r->SetDrawColor(Color(255, 255, 255, 255));
        r->ClearScreen();
        
        std::string fps = "Frames Per Second: " + std::to_string(t->GetFPS());
        font->Write(r->GetRenderer(), fps.c_str(), SDL_Color{ 0,0,255 }, SDL_Point{ 0,0 });
        vector<SDL_DisplayMode> modes = r->GetResolutions();
        for (int count = 0; count < modes.size(); count++)
        {
            stringstream s;
            s << modes[count].w << "x" << modes[count].h << " " << modes[count].refresh_rate << "hz";
            font->Write(r->GetRenderer(), s.str().c_str(), SDL_Color{ 255,128,0 }, SDL_Point{ 0,20 + count * 20 });
        }

        SDL_RenderPresent(r->GetRenderer());

        t->CapFPS();
    }

    delete SpriteAnim::Pool;
    delete SpriteSheet::Pool;
    
    font->Shutdown();
    r->Shutdown();
}
