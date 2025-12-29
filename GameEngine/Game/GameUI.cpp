#include "../Game/GameUI.h"
#include "../Graphics/Renderer.h"
#include "../Graphics/TTFont.h"
#include <sstream>

GameUI::GameUI()
{
    m_titleFont = nullptr;
    m_font = nullptr;
    m_state = UIState::StartScreen;
    m_startButtonRect = { 0, 0, 0, 0 };
    m_exitButtonRect = { 0, 0, 0, 0 };
    m_restartButtonRect = { 0, 0, 0, 0 };
    m_startRequested = false;
    m_restartRequested = false;
    m_exitRequested = false;
    m_startHovered = false;
    m_exitHovered = false;
    m_restartHovered = false;
}

GameUI::~GameUI()
{
    delete m_titleFont;
    m_titleFont = nullptr;
    delete m_font;
    m_font = nullptr;
}

void GameUI::Initialize()
{
    m_titleFont = new TTFont();
    m_titleFont->Initialize(20);
    
    m_font = new TTFont();
    m_font->Initialize(12);
}

void GameUI::SetState(UIState _state)
{
    m_state = _state;
    ResetRequests();
}

void GameUI::ResetRequests()
{
    m_startRequested = false;
    m_restartRequested = false;
    m_exitRequested = false;
    m_startHovered = false;
    m_exitHovered = false;
    m_restartHovered = false;
}

void GameUI::RenderButton(Renderer* _renderer, SDL_Rect& _rect, const char* _text, bool _hovered, bool _isExit)
{
    // Button background
    if (_isExit)
    {
        if (_hovered)
            SDL_SetRenderDrawColor(_renderer->GetRenderer(), 180, 80, 80, 255);
        else
            SDL_SetRenderDrawColor(_renderer->GetRenderer(), 150, 50, 50, 255);
    }
    else
    {
        if (_hovered)
            SDL_SetRenderDrawColor(_renderer->GetRenderer(), 80, 180, 80, 255);
        else
            SDL_SetRenderDrawColor(_renderer->GetRenderer(), 50, 150, 50, 255);
    }
    SDL_RenderFillRect(_renderer->GetRenderer(), &_rect);
    
    // Button border
    SDL_SetRenderDrawColor(_renderer->GetRenderer(), 255, 255, 255, 255);
    SDL_RenderDrawRect(_renderer->GetRenderer(), &_rect);
    
    // Button text centered
    int textW, textH;
    m_font->GetTextSize(_text, &textW, &textH);
    SDL_Color white = { 255, 255, 255, 255 };
    SDL_Point textPos = { _rect.x + (_rect.w - textW) / 2, _rect.y + (_rect.h - textH) / 2 };
    m_font->Write(_renderer->GetRenderer(), _text, white, textPos);
}

void GameUI::Render(Renderer* _renderer, int _score)
{
    switch (m_state)
    {
        case UIState::StartScreen:
            RenderStartScreen(_renderer);
            break;
        case UIState::Playing:
            RenderPlayingUI(_renderer, _score);
            break;
        case UIState::GameOver:
            RenderPlayingUI(_renderer, _score);
            RenderGameOver(_renderer, _score);
            break;
    }
}

void GameUI::RenderStartScreen(Renderer* _renderer)
{
    // Reset viewport to ensure UI renders in full logical space
    SDL_RenderSetViewport(_renderer->GetRenderer(), NULL);
    
    Point logicalSize = _renderer->GetLogicalSize();
    int centerX = logicalSize.X / 2;
    int centerY = logicalSize.Y / 2;
    
    // Draw background
    SDL_SetRenderDrawColor(_renderer->GetRenderer(), 30, 30, 50, 255);
    SDL_Rect bg = { 0, 0, logicalSize.X, logicalSize.Y };
    SDL_RenderFillRect(_renderer->GetRenderer(), &bg);
    
    // Draw title centered above buttons
    int titleW, titleH;
    m_titleFont->GetTextSize("Endless Game", &titleW, &titleH);
    SDL_Color yellow = { 255, 220, 50, 255 };
    SDL_Point titlePos = { centerX - titleW / 2, centerY - 60 };
    m_titleFont->Write(_renderer->GetRenderer(), "Endless Game", yellow, titlePos);
    
    // Buttons - START button at center
    int buttonWidth = 80;
    int buttonHeight = 22;
    
    m_startButtonRect = { centerX - buttonWidth / 2, centerY - buttonHeight / 2, buttonWidth, buttonHeight };
    RenderButton(_renderer, m_startButtonRect, "START", m_startHovered, false);
    
    m_exitButtonRect = { centerX - buttonWidth / 2, centerY + 35, buttonWidth, buttonHeight };
    RenderButton(_renderer, m_exitButtonRect, "EXIT", m_exitHovered, true);
}

void GameUI::RenderPlayingUI(Renderer* _renderer, int _score)
{
    // Render score
    std::stringstream ss;
    ss << "Score: " << _score;
    SDL_Color black = { 0, 0, 0, 255 };
    SDL_Point scorePos = { 10, 10 };
    m_font->Write(_renderer->GetRenderer(), ss.str().c_str(), black, scorePos);
}

void GameUI::RenderGameOver(Renderer* _renderer, int _score)
{
    // Reset viewport to ensure UI renders in full logical space
    SDL_RenderSetViewport(_renderer->GetRenderer(), NULL);
    
    Point logicalSize = _renderer->GetLogicalSize();
    int centerX = logicalSize.X / 2;
    int centerY = logicalSize.Y / 2;
    
    // Draw semi-transparent overlay
    SDL_SetRenderDrawBlendMode(_renderer->GetRenderer(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(_renderer->GetRenderer(), 0, 0, 0, 180);
    SDL_Rect overlay = { 0, 0, logicalSize.X, logicalSize.Y };
    SDL_RenderFillRect(_renderer->GetRenderer(), &overlay);
    
    // Draw "GAME OVER" title
    int titleW, titleH;
    m_font->GetTextSize("GAME OVER", &titleW, &titleH);
    SDL_Color red = { 255, 50, 50, 255 };
    SDL_Point titlePos = { centerX - titleW / 2, centerY - 50 };
    m_font->Write(_renderer->GetRenderer(), "GAME OVER", red, titlePos);
    
    // Draw final score
    std::stringstream ss;
    ss << "Score: " << _score;
    int scoreW, scoreH;
    m_font->GetTextSize(ss.str().c_str(), &scoreW, &scoreH);
    SDL_Color white = { 255, 255, 255, 255 };
    SDL_Point scorePos = { centerX - scoreW / 2, centerY - 30 };
    m_font->Write(_renderer->GetRenderer(), ss.str().c_str(), white, scorePos);
    
    // Buttons
    int buttonWidth = 80;
    int buttonHeight = 22;
    
    m_restartButtonRect = { centerX - buttonWidth / 2, centerY + 5, buttonWidth, buttonHeight };
    RenderButton(_renderer, m_restartButtonRect, "RESTART", m_restartHovered, false);
    
    m_exitButtonRect = { centerX - buttonWidth / 2, centerY + 35, buttonWidth, buttonHeight };
    RenderButton(_renderer, m_exitButtonRect, "EXIT", m_exitHovered, true);
}

void GameUI::HandleInput(SDL_Event& _event, Renderer* _renderer)
{
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    
    // Get actual window and logical sizes from renderer
    Point windowSize = _renderer->GetWindowSize();
    Point logicalSize = _renderer->GetLogicalSize();
    
    // Convert to logical coordinates using actual sizes
    mouseX = (int)(mouseX * (float)logicalSize.X / (float)windowSize.X);
    mouseY = (int)(mouseY * (float)logicalSize.Y / (float)windowSize.Y);
    
    // Update hover states based on current UI state
    if (m_state == UIState::StartScreen)
    {
        m_startHovered = (mouseX >= m_startButtonRect.x && 
                          mouseX <= m_startButtonRect.x + m_startButtonRect.w &&
                          mouseY >= m_startButtonRect.y && 
                          mouseY <= m_startButtonRect.y + m_startButtonRect.h);
        
        m_exitHovered = (mouseX >= m_exitButtonRect.x && 
                         mouseX <= m_exitButtonRect.x + m_exitButtonRect.w &&
                         mouseY >= m_exitButtonRect.y && 
                         mouseY <= m_exitButtonRect.y + m_exitButtonRect.h);
        
        if (_event.type == SDL_MOUSEBUTTONDOWN && _event.button.button == SDL_BUTTON_LEFT)
        {
            if (m_startHovered) m_startRequested = true;
            if (m_exitHovered) m_exitRequested = true;
        }
        
        if (_event.type == SDL_KEYDOWN)
        {
            if (_event.key.keysym.sym == SDLK_RETURN || _event.key.keysym.sym == SDLK_SPACE)
                m_startRequested = true;
            if (_event.key.keysym.sym == SDLK_ESCAPE)
                m_exitRequested = true;
        }
    }
    else if (m_state == UIState::GameOver)
    {
        m_restartHovered = (mouseX >= m_restartButtonRect.x && 
                            mouseX <= m_restartButtonRect.x + m_restartButtonRect.w &&
                            mouseY >= m_restartButtonRect.y && 
                            mouseY <= m_restartButtonRect.y + m_restartButtonRect.h);
        
        m_exitHovered = (mouseX >= m_exitButtonRect.x && 
                         mouseX <= m_exitButtonRect.x + m_exitButtonRect.w &&
                         mouseY >= m_exitButtonRect.y && 
                         mouseY <= m_exitButtonRect.y + m_exitButtonRect.h);
        
        if (_event.type == SDL_MOUSEBUTTONDOWN && _event.button.button == SDL_BUTTON_LEFT)
        {
            if (m_restartHovered) m_restartRequested = true;
            if (m_exitHovered) m_exitRequested = true;
        }
        
        if (_event.type == SDL_KEYDOWN)
        {
            if (_event.key.keysym.sym == SDLK_r || _event.key.keysym.sym == SDLK_RETURN)
                m_restartRequested = true;
            if (_event.key.keysym.sym == SDLK_ESCAPE)
                m_exitRequested = true;
        }
    }
}
