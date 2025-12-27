#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include "../Core/StandardIncludes.h"
#include "../Audio/AudioController.h"

class Renderer;
class TTFont;
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
    // Collision detection helper
    bool CheckAABBCollision(float _x1, float _y1, float _w1, float _h1,
                           float _x2, float _y2, float _w2, float _h2);
    void CheckPlayerCoinCollisions();
    void CheckPlayerEnemyCollisions();

    //Members
    SDL_Event m_sdlEvent;
    Renderer* m_renderer;
    InputController* m_input;
    Player* m_player;
    GameMap* g_Map;
    Camera* m_camera;
    TTFont* m_font;
    vector<Coin*> m_coins;
    vector<Enemy*> m_enemies;
    int m_score;

    bool m_quit;
};

#endif // GAME_CONTROLLER_H
