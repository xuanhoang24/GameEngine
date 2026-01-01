#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include "../Core/StandardIncludes.h"
#include "../Game/EntityManager.h"

class Renderer;
class InputController;
class ChunkMap;
class Camera;
class GameUI;

class GameController : public Singleton<GameController>
{
public:
    GameController();
    ~GameController();

    void RunGame();

private:
    void Initialize();
    void ShutDown();
    void HandleInput(SDL_Event& e);
    void RestartGame();

    SDL_Event m_event;
    Renderer* m_renderer = nullptr;
    InputController* m_input = nullptr;
    EntityManager m_entityManager;
    Entity* m_player = nullptr;
    ChunkMap* m_chunkMap = nullptr;
    Camera* m_camera = nullptr;
    GameUI* m_gameUI = nullptr;
    int m_score = 0;
    bool m_quit = false;
    bool m_collisionBoxDebug = false;
};

#endif
