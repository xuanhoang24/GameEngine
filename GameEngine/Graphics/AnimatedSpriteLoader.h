#ifndef ANIMATEDSPRITELOADER_H
#define ANIMATEDSPRITELOADER_H

#include "../Graphics/Texture.h"
#include "../Graphics/SpriteAnim.h"
#include "../Core/BasicStructs.h"

using namespace std;

struct AnimationData
{
    Texture* texture;
    SpriteAnim* animation;
    byte rows;
    byte columns;
    byte clipSizeX;
    byte clipSizeY;
    short frameCount;
};

class AnimatedSpriteLoader
{
public:
    // Constructors / Destructors
    AnimatedSpriteLoader();
    virtual ~AnimatedSpriteLoader();

    //Methods
    void LoadAnimation(const string& _animationName, const string& _filePath, 
                      byte _rows, byte _columns, byte _clipSizeX, byte _clipSizeY, 
                      short _frameCount, float _animSpeed);
    
    void LoadAnimationAuto(const string& _animationName, const string& _filePath,
                          byte _clipSizeX, byte _clipSizeY, float _animSpeed);
    
    Rect UpdateAnimation(const string& _animationName, float _deltaTime);
    
    Texture* GetTexture(const string& _animationName);
    
    int GetCurrentFrame(const string& _animationName);
    
    void SetCurrentAnimation(const string& _animationName);
    
    string GetCurrentAnimation() { return m_currentAnimation; }
    
    void ClearAnimations();
    
    bool HasAnimation(const string& _animationName);

private:
    map<string, AnimationData> m_animations;
    string m_currentAnimation;
};

#endif // ANIMATEDSPRITELOADER_H
