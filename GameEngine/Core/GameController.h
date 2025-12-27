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
class GameMap;
class Camera;
class Coin;
class Enemy;
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
    //Members
    SDL_Event m_sdlEvent;
    Renderer* m_renderer;
    InputController* m_input;
    Player* m_player;
    GameMap* g_Map;
    Camera* m_camera;
    vector<Coin*> m_coins;
    vector<Enemy*> m_enemies;

    bool m_quit;
};

#endif // GAME_CONTROLLER_H
