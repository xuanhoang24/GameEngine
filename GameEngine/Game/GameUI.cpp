#include "../Game/GameUI.h"
#include "../Graphics/Renderer.h"
#include "../Graphics/TTFont.h"
#include "../Graphics/Texture.h"
#include "../Graphics/AnimatedSpriteLoader.h"
#include "../Core/Timing.h"
#include "../Audio/GameAudioManager.h"
#include <sstream>

GameUI::GameUI()
{
    m_titleFont = nullptr;
    m_font = nullptr;
    m_heartAnimLoader = nullptr;
    m_cameraX = 0.0f;
    m_state = UIState::StartScreen;
    m_keyStartRequested = false;
    m_keyExitRequested = false;
    m_keyRestartRequested = false;
    m_keyResumeRequested = false;
}

GameUI::~GameUI()
{
    delete m_titleFont;
    m_titleFont = nullptr;
    delete m_font;
    m_font = nullptr;
    delete m_heartAnimLoader;
    m_heartAnimLoader = nullptr;

    for (auto& bg : m_backgroundLayers)
        if (bg.texture) SDL_DestroyTexture(bg.texture);
    m_backgroundLayers.clear();
}

void GameUI::Initialize()
{
    m_titleFont = new TTFont();
    m_titleFont->Initialize(20);

    m_font = new TTFont();
    m_font->Initialize(12);

    m_heartAnimLoader = new AnimatedSpriteLoader();
    m_heartAnimLoader->LoadAnimation("heart_full", "../Assets/Textures/Player/heart2-shine.png", 1, 6, 16, 16, 6, 2.0f);
    m_heartAnimLoader->LoadAnimation("heart_empty", "../Assets/Textures/Player/heart2-empty.png", 1, 1, 16, 16, 6, 0.0f);

    // Initialize background layers
    AddBackgroundLayer("../Assets/Maps/Images/Tilemap/Background_2.png", 0.3f);
    AddBackgroundLayer("../Assets/Maps/Images/Tilemap/Background_1.png", 0.6f);

    // Initialize volume slider with current volume
    m_volumeSlider.SetValue(GameAudioManager::Instance().GetMasterVolume());
}

void GameUI::AddBackgroundLayer(const string& _path, float _parallaxFactor)
{
    SDL_Renderer* sdl = Renderer::Instance().GetRenderer();
    SDL_Texture* texture = IMG_LoadTexture(sdl, _path.c_str());

    if (texture)
    {
        BackgroundLayer layer;
        layer.texture = texture;
        layer.parallaxFactor = _parallaxFactor;
        SDL_QueryTexture(texture, nullptr, nullptr, &layer.width, &layer.height);
        m_backgroundLayers.push_back(layer);
    }
}

void GameUI::Update(float _deltaTime)
{
    // Update camera position for parallax scrolling only on start screen
    if (m_state == UIState::StartScreen)
    {
        m_cameraX += 60.0f * _deltaTime;
    }
}

void GameUI::SetState(UIState _state)
{
    m_state = _state;
    ResetRequests();
}

void GameUI::ResetRequests()
{
    m_startButton.Reset();
    m_exitButton.Reset();
    m_restartButton.Reset();
    m_resumeButton.Reset();
    m_mainMenuButton.Reset();
    m_volumeSlider.Reset();
    m_keyStartRequested = false;
    m_keyExitRequested = false;
    m_keyRestartRequested = false;
    m_keyResumeRequested = false;
}

void GameUI::ConvertToLogicalCoords(Renderer* _renderer, int& mouseX, int& mouseY)
{
    Point windowSize = _renderer->GetWindowSize();
    Point logicalSize = _renderer->GetLogicalSize();
    mouseX = (int)(mouseX * (float)logicalSize.X / (float)windowSize.X);
    mouseY = (int)(mouseY * (float)logicalSize.Y / (float)windowSize.Y);
}

void GameUI::RenderParallaxBackground(Renderer* _renderer)
{
    SDL_Renderer* sdl = _renderer->GetRenderer();

    Point logicalSize = _renderer->GetLogicalSize();
    int screenWidth = logicalSize.X;
    int screenHeight = logicalSize.Y;

    for (const auto& layer : m_backgroundLayers)
    {
        if (!layer.texture || layer.width == 0) continue;

        float scale = (float)screenHeight / (float)layer.height;
        int scaledWidth = (int)(layer.width * scale);
        int scaledHeight = screenHeight;

        if (scaledWidth == 0) continue;

        float parallaxOffset = m_cameraX * layer.parallaxFactor;
        float offset = fmod(parallaxOffset, (float)scaledWidth);
        float startX = -offset;

        while (startX > 0) startX -= scaledWidth;

        for (float x = startX; x < screenWidth + scaledWidth; x += scaledWidth)
        {
            SDL_Rect dst = { (int)x, 0, scaledWidth, scaledHeight };
            SDL_RenderCopy(sdl, layer.texture, nullptr, &dst);
        }
    }
}

void GameUI::Render(Renderer* _renderer, int _score, int _health, int _maxHealth)
{
    switch (m_state)
    {
    case UIState::StartScreen:
        RenderStartScreen(_renderer);
        break;
    case UIState::Playing:
        RenderPlayingUI(_renderer, _score, _health, _maxHealth);
        break;
    case UIState::Paused:
        RenderPlayingUI(_renderer, _score, _health, _maxHealth);
        RenderPauseMenu(_renderer);
        break;
    case UIState::GameOver:
        RenderPlayingUI(_renderer, _score, _health, _maxHealth);
        RenderGameOver(_renderer, _score);
        break;
    }
}

void GameUI::RenderStartScreen(Renderer* _renderer)
{
    SDL_RenderSetViewport(_renderer->GetRenderer(), NULL);

    Point logicalSize = _renderer->GetLogicalSize();
    int centerX = logicalSize.X / 2;
    int centerY = logicalSize.Y / 2;

    // Render parallax background
    RenderParallaxBackground(_renderer);

    // Draw overlay
    SDL_SetRenderDrawBlendMode(_renderer->GetRenderer(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(_renderer->GetRenderer(), 0, 0, 0, 180);
    SDL_Rect overlay = { 0, 0, (int)logicalSize.X, (int)logicalSize.Y };
    SDL_RenderFillRect(_renderer->GetRenderer(), &overlay);

    // Draw title
    int titleW, titleH;
    m_titleFont->GetTextSize("Platformer Game", &titleW, &titleH);
    SDL_Color yellow = { 255, 220, 50, 255 };
    SDL_Point titlePos = { centerX - titleW / 2, centerY - 60 };
    m_titleFont->Write(_renderer->GetRenderer(), "Platformer Game", yellow, titlePos);

    // Buttons
    int buttonWidth = 90;
    int buttonHeight = 18;

    // Start button
    m_startButton.SetRect(centerX - buttonWidth / 2, centerY - 30, buttonWidth, buttonHeight);
    m_startButton.Render(_renderer, m_font, "START", false);

    // Volume slider
    int sliderWidth = 100;
    int sliderHeight = 12;
    m_volumeSlider.SetRect(centerX - sliderWidth / 2, centerY + 5, sliderWidth, sliderHeight);
    m_volumeSlider.Render(_renderer, m_font, "Volume");

    // Exit button
    m_exitButton.SetRect(centerX - buttonWidth / 2, centerY + 35, buttonWidth, buttonHeight);
    m_exitButton.Render(_renderer, m_font, "EXIT", true);
}

void GameUI::RenderPlayingUI(Renderer* _renderer, int _score, int _health, int _maxHealth)
{
    std::stringstream ss;
    ss << "Score: " << _score;
    SDL_Color black = { 0, 0, 0, 255 };
    SDL_Point scorePos = { 10, 10 };
    m_font->Write(_renderer->GetRenderer(), ss.str().c_str(), black, scorePos);

    // Render FPS
    std::stringstream fpsStream;
    fpsStream << "FPS: " << Timing::Instance().GetFPS();
    Point logicalSize = _renderer->GetLogicalSize();
    SDL_Point fpsPos = { (int)logicalSize.X - 45, 10 };
    m_font->Write(_renderer->GetRenderer(), 8, fpsStream.str().c_str(), black, fpsPos);

    RenderHearts(_renderer, _health, _maxHealth);
}

void GameUI::RenderHearts(Renderer* _renderer, int _health, int _maxHealth)
{
    int heartSize = 16;
    int heartSpacing = 2;
    int startX = 10;
    int startY = 26;

    float deltaTime = Timing::Instance().GetDeltaTime();

    for (int i = 0; i < _maxHealth; i++)
    {
        string animName = (i < _health) ? "heart_full" : "heart_empty";

        Rect srcRect = m_heartAnimLoader->UpdateAnimation(animName, deltaTime);
        Texture* heartTex = m_heartAnimLoader->GetTexture(animName);

        if (heartTex)
        {
            Rect destRect(
                startX + i * (heartSize + heartSpacing),
                startY,
                startX + i * (heartSize + heartSpacing) + heartSize,
                startY + heartSize
            );
            _renderer->RenderTexture(heartTex, srcRect, destRect);
        }
    }
}

void GameUI::RenderPauseMenu(Renderer* _renderer)
{
    SDL_RenderSetViewport(_renderer->GetRenderer(), NULL);

    Point logicalSize = _renderer->GetLogicalSize();
    int centerX = logicalSize.X / 2;
    int centerY = logicalSize.Y / 2;

    // Draw overlay
    SDL_SetRenderDrawBlendMode(_renderer->GetRenderer(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(_renderer->GetRenderer(), 0, 0, 0, 180);
    SDL_Rect overlay = { 0, 0, (int)logicalSize.X, (int)logicalSize.Y };
    SDL_RenderFillRect(_renderer->GetRenderer(), &overlay);

    // Draw title
    int titleW, titleH;
    m_titleFont->GetTextSize("PAUSED", &titleW, &titleH);
    SDL_Color yellow = { 255, 220, 50, 255 };
    SDL_Point titlePos = { centerX - titleW / 2, centerY - 70 };
    m_titleFont->Write(_renderer->GetRenderer(), "PAUSED", yellow, titlePos);

    // Buttons
    int buttonWidth = 90;
    int buttonHeight = 18;

    // Resume button
    m_resumeButton.SetRect(centerX - buttonWidth / 2, centerY - 35, buttonWidth, buttonHeight);
    m_resumeButton.Render(_renderer, m_font, "RESUME", false);

    // Main Menu button
    m_mainMenuButton.SetRect(centerX - buttonWidth / 2, centerY - 10, buttonWidth, buttonHeight);
    m_mainMenuButton.Render(_renderer, m_font, "MAIN MENU", false);

    // Volume slider
    int sliderWidth = 100;
    int sliderHeight = 12;
    m_volumeSlider.SetRect(centerX - sliderWidth / 2, centerY + 25, sliderWidth, sliderHeight);
    m_volumeSlider.Render(_renderer, m_font, "Volume");

    // Exit button
    m_exitButton.SetRect(centerX - buttonWidth / 2, centerY + 55, buttonWidth, buttonHeight);
    m_exitButton.Render(_renderer, m_font, "EXIT", true);
}

void GameUI::RenderGameOver(Renderer* _renderer, int _score)
{
    SDL_RenderSetViewport(_renderer->GetRenderer(), NULL);

    Point logicalSize = _renderer->GetLogicalSize();
    int centerX = logicalSize.X / 2;
    int centerY = logicalSize.Y / 2;

    // Draw overlay
    SDL_SetRenderDrawBlendMode(_renderer->GetRenderer(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(_renderer->GetRenderer(), 0, 0, 0, 180);
    SDL_Rect overlay = { 0, 0, (int)logicalSize.X, (int)logicalSize.Y };
    SDL_RenderFillRect(_renderer->GetRenderer(), &overlay);

    // Draw title
    int titleW, titleH;
    m_font->GetTextSize("GAME OVER", &titleW, &titleH);
    SDL_Color red = { 255, 50, 50, 255 };
    SDL_Point titlePos = { centerX - titleW / 2, centerY - 50 };
    m_font->Write(_renderer->GetRenderer(), "GAME OVER", red, titlePos);

    // Draw score
    std::stringstream ss;
    ss << "Score: " << _score;
    int scoreW, scoreH;
    m_font->GetTextSize(ss.str().c_str(), &scoreW, &scoreH);
    SDL_Color white = { 255, 255, 255, 255 };
    SDL_Point scorePos = { centerX - scoreW / 2, centerY - 30 };
    m_font->Write(_renderer->GetRenderer(), ss.str().c_str(), white, scorePos);

    // Buttons
    int buttonWidth = 90;
    int buttonHeight = 18;

    m_restartButton.SetRect(centerX - buttonWidth / 2, centerY, buttonWidth, buttonHeight);
    m_restartButton.Render(_renderer, m_font, "RESTART", false);

    m_mainMenuButton.SetRect(centerX - buttonWidth / 2, centerY + 25, buttonWidth, buttonHeight);
    m_mainMenuButton.Render(_renderer, m_font, "MAIN MENU", false);

    m_exitButton.SetRect(centerX - buttonWidth / 2, centerY + 50, buttonWidth, buttonHeight);
    m_exitButton.Render(_renderer, m_font, "EXIT", true);
}


void GameUI::HandleInput(SDL_Event& _event, Renderer* _renderer)
{
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    ConvertToLogicalCoords(_renderer, mouseX, mouseY);

    if (m_state == UIState::StartScreen)
    {
        m_startButton.UpdateHover(mouseX, mouseY);
        m_exitButton.UpdateHover(mouseX, mouseY);
        m_volumeSlider.UpdateHover(mouseX, mouseY);

        m_startButton.CheckClick(_event);
        m_exitButton.CheckClick(_event);

        // Handle volume slider
        if (m_volumeSlider.HandleInput(_event, mouseX, mouseY))
        {
            GameAudioManager::Instance().SetMasterVolume(m_volumeSlider.GetValue());
        }

        if (_event.type == SDL_KEYDOWN)
        {
            if (_event.key.keysym.sym == SDLK_RETURN || _event.key.keysym.sym == SDLK_SPACE)
                m_keyStartRequested = true;
            if (_event.key.keysym.sym == SDLK_ESCAPE)
                m_keyExitRequested = true;
        }
    }
    else if (m_state == UIState::Playing)
    {
        if (_event.type == SDL_KEYDOWN && _event.key.keysym.sym == SDLK_ESCAPE)
        {
            SetState(UIState::Paused);
        }
    }
    else if (m_state == UIState::Paused)
    {
        m_resumeButton.UpdateHover(mouseX, mouseY);
        m_mainMenuButton.UpdateHover(mouseX, mouseY);
        m_exitButton.UpdateHover(mouseX, mouseY);
        m_volumeSlider.UpdateHover(mouseX, mouseY);

        m_resumeButton.CheckClick(_event);
        m_mainMenuButton.CheckClick(_event);
        m_exitButton.CheckClick(_event);

        // Handle volume slider
        if (m_volumeSlider.HandleInput(_event, mouseX, mouseY))
        {
            GameAudioManager::Instance().SetMasterVolume(m_volumeSlider.GetValue());
        }

        if (_event.type == SDL_KEYDOWN && _event.key.keysym.sym == SDLK_ESCAPE)
        {
            m_keyResumeRequested = true;
        }
    }
    else if (m_state == UIState::GameOver)
    {
        m_restartButton.UpdateHover(mouseX, mouseY);
        m_mainMenuButton.UpdateHover(mouseX, mouseY);
        m_exitButton.UpdateHover(mouseX, mouseY);

        m_restartButton.CheckClick(_event);
        m_mainMenuButton.CheckClick(_event);
        m_exitButton.CheckClick(_event);

        if (_event.type == SDL_KEYDOWN)
        {
            if (_event.key.keysym.sym == SDLK_r || _event.key.keysym.sym == SDLK_RETURN)
                m_keyRestartRequested = true;
            if (_event.key.keysym.sym == SDLK_ESCAPE)
                m_keyExitRequested = true;
        }
    }
}
