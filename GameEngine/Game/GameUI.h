#ifndef GAME_UI_H
#define GAME_UI_H

#include "../Core/StandardIncludes.h"
#include "../Game/UIButton.h"

class Renderer;
class TTFont;
class Texture;
class Player;
class AnimatedSpriteLoader;

enum class UIState
{
    StartScreen,
    Playing,
    Paused,
    GameOver
};

class GameUI
{
public:
    GameUI();
    virtual ~GameUI();

    void Initialize();
    void Render(Renderer* _renderer, int _score, Player* _player);
    void HandleInput(SDL_Event& _event, Renderer* _renderer);
    
    UIState GetState() const { return m_state; }
    void SetState(UIState _state);
    
    bool IsStartRequested() const { return m_startButton.IsClicked() || m_keyStartRequested; }
    bool IsRestartRequested() const { return m_restartButton.IsClicked() || m_keyRestartRequested; }
    bool IsExitRequested() const { return m_exitButton.IsClicked() || m_keyExitRequested; }
    bool IsResumeRequested() const { return m_resumeButton.IsClicked() || m_keyResumeRequested; }
    bool IsMainMenuRequested() const { return m_mainMenuButton.IsClicked(); }
    
    void ResetRequests();

private:
    void RenderStartScreen(Renderer* _renderer);
    void RenderPlayingUI(Renderer* _renderer, int _score, Player* _player);
    void RenderPauseMenu(Renderer* _renderer);
    void RenderGameOver(Renderer* _renderer, int _score);
    void RenderHearts(Renderer* _renderer, int _health, int _maxHealth);
    
    // Convert screen coordinates to logical coordinates
    void ConvertToLogicalCoords(Renderer* _renderer, int& mouseX, int& mouseY);
    
    TTFont* m_titleFont;
    TTFont* m_font;
    
    // Heart animation
    AnimatedSpriteLoader* m_heartAnimLoader;
    
    UIState m_state;
    
    // UI Buttons
    UIButton m_startButton;
    UIButton m_exitButton;
    UIButton m_restartButton;
    UIButton m_resumeButton;
    UIButton m_mainMenuButton;
    
    // For keyboard shortcuts
    bool m_keyStartRequested;
    bool m_keyExitRequested;
    bool m_keyRestartRequested;
    bool m_keyResumeRequested;
};

#endif // GAME_UI_H
