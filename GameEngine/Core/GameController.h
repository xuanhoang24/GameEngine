#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include "../Core/StandardIncludes.h"
#include "../Audio/AudioController.h"

class Renderer;
class TTFont;
class InputController;
class AudioController;
class SoundEffect;
class Song;
class WavDraw;
class Player;
class ChunkMap;
class Camera;
class GameUI;

class GameController : public Singleton<GameController>
{
public:
    // Constructors/ Destructors
    GameController();
    virtual ~GameController();

    //Methods
    void RunGame();
    void Initialize();
    void HandleInput(SDL_Event _event);
    void ShutDown();

private:
    void RestartGame();

    //Members
    SDL_Event m_sdlEvent;
    Renderer* m_renderer;
    InputController* m_input;
    Player* m_player;
    ChunkMap* m_chunkMap;
    Camera* m_camera;
    GameUI* m_gameUI;
    int m_score;

    bool m_quit;
};

#endif // GAME_CONTROLLER_H
