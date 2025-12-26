#include "../Graphics/AnimatedSpriteLoader.h"
#include <iostream>

AnimatedSpriteLoader::AnimatedSpriteLoader()
{
    m_currentAnimation = "";
}

AnimatedSpriteLoader::~AnimatedSpriteLoader()
{
    ClearAnimations();
}

void AnimatedSpriteLoader::LoadAnimation(const string& _animationName, const string& _filePath,
    byte _rows, byte _columns, byte _clipSizeX, byte _clipSizeY,
    short _frameCount, float _animSpeed)
{
    // Create texture
    Texture* texture = Texture::Pool->GetResource();
    texture->Load(_filePath);

    // Create animation
    SpriteAnim* anim = SpriteAnim::Pool->GetResource();
    anim->Create(0, _frameCount, _animSpeed);

    // Store animation data
    AnimationData data;
    data.texture = texture;
    data.animation = anim;
    data.rows = _rows;
    data.columns = _columns;
    data.clipSizeX = _clipSizeX;
    data.clipSizeY = _clipSizeY;
    data.frameCount = _frameCount;

    m_animations[_animationName] = data;

    // Set as current if first animation
    if (m_currentAnimation.empty())
    {
        m_currentAnimation = _animationName;
    }
}

void AnimatedSpriteLoader::LoadAnimationAuto(const string& _animationName, const string& _filePath,
    byte _clipSizeX, byte _clipSizeY, float _animSpeed)
{
    // Create texture first to get dimensions
    Texture* texture = Texture::Pool->GetResource();
    texture->Load(_filePath);

    ImageInfo* info = texture->GetImageInfo();
    
    // Calculate rows and columns automatically
    byte columns = info->Width / _clipSizeX;
    byte rows = info->Height / _clipSizeY;
    short frameCount = columns * rows;

    // Create animation
    SpriteAnim* anim = SpriteAnim::Pool->GetResource();
    anim->Create(0, frameCount, _animSpeed);

    // Store animation data
    AnimationData data;
    data.texture = texture;
    data.animation = anim;
    data.rows = rows;
    data.columns = columns;
    data.clipSizeX = _clipSizeX;
    data.clipSizeY = _clipSizeY;
    data.frameCount = frameCount;

    m_animations[_animationName] = data;

    // Set as current if first animation
    if (m_currentAnimation.empty())
    {
        m_currentAnimation = _animationName;
    }
}

Rect AnimatedSpriteLoader::UpdateAnimation(const string& _animationName, float _deltaTime)
{
    // Safety check for empty animation name
    if (_animationName.empty())
    {
        return Rect(0, 0, 0, 0);
    }

    // Check if animation exists
    auto it = m_animations.find(_animationName);
    if (it == m_animations.end())
    {
        return Rect(0, 0, 0, 0);
    }

    AnimationData& data = it->second;
    
    // Safety check for null animation
    if (!data.animation)
    {
        return Rect(0, 0, 0, 0);
    }
    
    // Get current frame
    short currentFrame = data.animation->GetClipCurrent();
    
    // Calculate position in sprite sheet
    short posX = (currentFrame % data.columns) * data.clipSizeX;
    short posY = (currentFrame / data.columns) * data.clipSizeY;
    
    Rect rect = Rect(posX, posY, posX + data.clipSizeX, posY + data.clipSizeY);

    // Update animation
    data.animation->Update(_deltaTime);

    return rect;
}

Texture* AnimatedSpriteLoader::GetTexture(const string& _animationName)
{
    // Safety check for empty animation name
    if (_animationName.empty())
    {
        return nullptr;
    }

    // Check if animation exists
    auto it = m_animations.find(_animationName);
    if (it == m_animations.end())
    {
        return nullptr;
    }
    
    return it->second.texture;
}

int AnimatedSpriteLoader::GetCurrentFrame(const string& _animationName)
{
    // Safety check for empty animation name
    if (_animationName.empty())
    {
        return 0;
    }

    // Check if animation exists
    auto it = m_animations.find(_animationName);
    if (it == m_animations.end())
    {
        return 0;
    }
    
    // Safety check for null animation
    if (!it->second.animation)
    {
        return 0;
    }
    
    return it->second.animation->GetClipCurrent();
}

void AnimatedSpriteLoader::SetCurrentAnimation(const string& _animationName)
{
    // Safety check for empty animation name
    if (_animationName.empty())
    {
        return;
    }

    // Check if animation exists
    if (m_animations.find(_animationName) != m_animations.end())
    {
        m_currentAnimation = _animationName;
    }
}

void AnimatedSpriteLoader::ClearAnimations()
{
    for (auto& pair : m_animations)
    {
        if (pair.second.texture)
        {
            Texture::Pool->ReleaseResource(pair.second.texture);
        }
        if (pair.second.animation)
        {
            SpriteAnim::Pool->ReleaseResource(pair.second.animation);
        }
    }
    m_animations.clear();
    m_currentAnimation = "";
}

bool AnimatedSpriteLoader::HasAnimation(const string& _animationName)
{
    // Safety check for empty animation name
    if (_animationName.empty())
    {
        return false;
    }
    
    return m_animations.find(_animationName) != m_animations.end();
}
