#ifndef GAME_UI_H
#define GAME_UI_H

#include "../Core/StandardIncludes.h"

class Renderer;
class TTFont;

enum class UIState
{
    StartScreen,
    Playing,
    GameOver
};

class GameUI
{
public:
    GameUI();
    virtual ~GameUI();

    void Initialize();
    void Render(Renderer* _renderer, int _score);
    void HandleInput(SDL_Event& _event, Renderer* _renderer);
    
    UIState GetState() const { return m_state; }
    void SetState(UIState _state);
    
    bool IsStartRequested() const { return m_startRequested; }
    bool IsRestartRequested() const { return m_restartRequested; }
    bool IsExitRequested() const { return m_exitRequested; }
    
    void ResetRequests();

private:
    void RenderStartScreen(Renderer* _renderer);
    void RenderPlayingUI(Renderer* _renderer, int _score);
    void RenderGameOver(Renderer* _renderer, int _score);
    void RenderButton(Renderer* _renderer, SDL_Rect& _rect, const char* _text, bool _hovered, bool _isExit);
    
    TTFont* m_titleFont;
    TTFont* m_font;
    
    UIState m_state;
    
    SDL_Rect m_startButtonRect;
    SDL_Rect m_exitButtonRect;
    SDL_Rect m_restartButtonRect;
    
    bool m_startRequested;
    bool m_restartRequested;
    bool m_exitRequested;
    
    bool m_startHovered;
    bool m_exitHovered;
    bool m_restartHovered;
};

#endif // GAME_UI_H
