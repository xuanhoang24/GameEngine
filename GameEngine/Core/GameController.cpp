#include "GameController.h"
#include "../Graphics/Renderer.h"
#include "../Graphics/Camera.h"
#include "../Graphics/SpriteAnim.h"
#include "../Graphics/SpriteSheet.h"
#include "../Graphics/Texture.h"
#include "../Input/InputController.h"
#include "../Audio/GameAudioManager.h"
#include "../Resources/AssetController.h"
#include "../Core/Timing.h"
#include "../Game/ChunkMap.h"
#include "../Game/GameUI.h"

GameController::GameController() { m_event = {}; }
GameController::~GameController() { ShutDown(); }

void GameController::Initialize()
{
    AssetController::Instance().Initialize(50000000);
    SpriteAnim::Pool = new ObjectPool<SpriteAnim>();
    SpriteSheet::Pool = new ObjectPool<SpriteSheet>();
    Texture::Pool = new ObjectPool<Texture>();

    m_renderer = &Renderer::Instance();
    m_renderer->Initialize();
    m_input = &InputController::Instance();
    m_camera = new Camera();

    GameAudioManager::Instance().Initialize();

    m_chunkMap = new ChunkMap();
    m_chunkMap->SetEntityManager(&m_entityManager);
    m_chunkMap->LoadDefaultChunks();
    m_renderer->SetLogicalSizeFromMapHeight(m_chunkMap->GetMapPixelHeight());

    float sx, sy;
    m_chunkMap->GetPlayerSpawnPoint(sx, sy);
    m_player = EntityFactory::CreatePlayer(sx, sy);
    m_entityManager.GetAllEntities().push_back(m_player);
    m_entityManager.SetChunkMap(m_chunkMap);

    m_gameUI = new GameUI();
    m_gameUI->Initialize();
    
    GameAudioManager::Instance().PlayMenuMusic();
}

void GameController::ShutDown()
{
    m_entityManager.Clear();
    m_player = nullptr;
    delete m_gameUI; m_gameUI = nullptr;
    delete m_camera; m_camera = nullptr;
    delete m_chunkMap; m_chunkMap = nullptr;
    delete SpriteAnim::Pool; SpriteAnim::Pool = nullptr;
    delete SpriteSheet::Pool; SpriteSheet::Pool = nullptr;
    delete Texture::Pool; Texture::Pool = nullptr;
}

void GameController::HandleInput(SDL_Event& e)
{
    if (e.type == SDL_QUIT) m_quit = true;
    
    // F1 toggles spatial grid debug visualization
    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_F1)
    {
        m_entityManager.ToggleSpatialGridDebug();
    }
    
    // F2 toggles collision box debug visualization (map + entities)
    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_F2)
    {
        m_collisionBoxDebug = !m_collisionBoxDebug;
        m_entityManager.ToggleCollisionBoxDebug();
    }

    m_gameUI->HandleInput(e, m_renderer);

    if (m_gameUI->IsStartRequested()) 
    {
        m_gameUI->SetState(UIState::Playing);
        GameAudioManager::Instance().PlayClickSound();
    }
    if (m_gameUI->IsRestartRequested()) 
    {
        RestartGame();
        GameAudioManager::Instance().PlayClickSound();
    }
    if (m_gameUI->IsResumeRequested()) 
    {
        m_gameUI->SetState(UIState::Playing);
        GameAudioManager::Instance().PlayClickSound();
    }
    if (m_gameUI->IsMainMenuRequested()) 
    { 
        RestartGame(); 
        m_gameUI->SetState(UIState::StartScreen);
        GameAudioManager::Instance().PlayClickSound();
    }
    if (m_gameUI->IsExitRequested()) 
    {
        m_quit = true;
        GameAudioManager::Instance().PlayClickSound();
    }
}

void GameController::RunGame()
{
    Timing& t = Timing::Instance();
    t.SetFPS(60);
    Initialize();

    while (!m_quit)
    {
        t.Tick();
        m_renderer->SetDrawColor(Color(255, 255, 255, 255));
        m_renderer->ClearScreen();

        while (SDL_PollEvent(&m_event)) HandleInput(m_event);

        m_gameUI->Update(t.GetDeltaTime());

        int hp = 0, maxHp = 3;
        bool dead = false, fullyDead = false;
        if (m_player)
        {
            auto* h = m_player->GetComponent<HealthComponent>();
            if (h) { hp = h->health; maxHp = h->maxHealth; dead = h->isDead; fullyDead = h->isFullyDead; }
        }

        UIState state = m_gameUI->GetState();
        
        // Handle audio state transitions
        if (state != m_previousState)
        {
            switch (state)
            {
                case UIState::StartScreen:
                case UIState::GameOver:
                    GameAudioManager::Instance().StopAllSounds();
                    GameAudioManager::Instance().PlayMenuMusic();
                    break;
                case UIState::Playing:
                    GameAudioManager::Instance().StopAllSounds();
                    GameAudioManager::Instance().PlayBackgroundMusic();
                    break;
                case UIState::Paused:
                    GameAudioManager::Instance().PauseMusic();
                    break;
            }
            
            // Resume music when returning to playing from pause
            if (m_previousState == UIState::Paused && state == UIState::Playing)
            {
                GameAudioManager::Instance().ResumeMusic();
            }
            
            m_previousState = state;
        }

        if (state == UIState::StartScreen)
        {
            m_gameUI->Render(m_renderer, m_score, 0, 3);
            t.CapFPS();
            SDL_RenderPresent(m_renderer->GetRenderer());
            continue;
        }

        if (state == UIState::Paused)
        {
            m_chunkMap->RenderBackgrounds(m_renderer, m_camera);
            m_chunkMap->Render(m_renderer, m_camera);
            m_entityManager.Render(m_renderer, m_camera);
            m_gameUI->Render(m_renderer, m_score, hp, maxHp);
            t.CapFPS();
            SDL_RenderPresent(m_renderer->GetRenderer());
            continue;
        }

        if (fullyDead && state == UIState::Playing)
            m_gameUI->SetState(UIState::GameOver);

        if (!dead)
        {
            Point sz = m_renderer->GetLogicalSize();
            m_entityManager.SetScrollParams(m_camera->GetX(), sz.X, m_chunkMap->GetChunkPixelWidth());
            m_entityManager.Update(t.GetDeltaTime());
            m_score += m_entityManager.GetScore();
            m_entityManager.ResetScore();

            if (m_player) m_camera->FollowEntity(m_player, m_renderer);
            m_chunkMap->Update(m_camera->GetX(), (float)sz.X);

            if (m_player)
            {
                auto* tr = m_player->GetComponent<TransformComponent>();
                if (tr && tr->worldX < m_camera->GetMaxX()) tr->worldX = m_camera->GetMaxX();
            }
        }
        else if (!fullyDead)
        {
            auto* h = m_player->GetComponent<HealthComponent>();
            auto* s = m_player->GetComponent<SpriteComponent>();
            if (h) { h->deathTimer += t.GetDeltaTime(); if (h->deathTimer >= h->deathDuration) h->isFullyDead = true; }
            if (s) s->currentAnimation = "hurt";
        }

        m_chunkMap->RenderBackgrounds(m_renderer, m_camera);
        m_chunkMap->Render(m_renderer, m_camera);
        if (!fullyDead) m_entityManager.Render(m_renderer, m_camera);
        
        // Render spatial grid debug overlay (F1)
        Point logicalSize = m_renderer->GetLogicalSize();
        m_entityManager.RenderSpatialGridDebug(m_renderer, m_camera, (float)logicalSize.X, (float)logicalSize.Y);
        
        // Render collision box debug overlay (F2)
        if (m_collisionBoxDebug)
        {
            m_chunkMap->RenderCollisionDebug(m_renderer, m_camera);
            m_entityManager.RenderCollisionBoxDebug(m_renderer, m_camera);
        }
        
        m_gameUI->Render(m_renderer, m_score, hp, maxHp);

        t.CapFPS();
        SDL_RenderPresent(m_renderer->GetRenderer());
    }
}

void GameController::RestartGame()
{
    for (auto* e : m_entityManager.GetAllEntities())
        if (e && e != m_player) m_entityManager.DestroyEntity(e);

    if (m_player)
    {
        float sx, sy;
        m_chunkMap->GetPlayerSpawnPoint(sx, sy);

        auto* t = m_player->GetComponent<TransformComponent>();
        if (t) { t->worldX = t->baseX = sx; t->worldY = t->baseY = sy; t->mapInstance = 0; }

        auto* m = m_player->GetComponent<MovementComponent>();
        if (m) m->velocityX = m->velocityY = 0;

        auto* h = m_player->GetComponent<HealthComponent>();
        if (h) { h->health = h->maxHealth; h->isDead = h->isFullyDead = h->isInvincible = false; h->deathTimer = h->invincibleTimer = 0; }

        auto* s = m_player->GetComponent<SpriteComponent>();
        if (s) { s->facingRight = true; s->flickering = false; s->flickerCounter = 0; }

        auto* j = m_player->GetComponent<JumpComponent>();
        if (j) j->isJumping = j->jumpPressed = false;

        m_player->SetActive(true);
    }

    m_chunkMap->Reset();
    m_camera->Reset();
    m_score = 0;
    m_gameUI->SetState(UIState::Playing);
}
