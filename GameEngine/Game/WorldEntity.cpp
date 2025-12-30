#include "../Game/WorldEntity.h"
#include "../Graphics/Camera.h"
#include "../Core/Timing.h"

WorldEntity::WorldEntity()
{
	m_animLoader = nullptr;
	m_worldX = 0.0f;
	m_worldY = 0.0f;
	m_baseX = 0.0f;
	m_baseY = 0.0f;
	m_isActive = true;
	m_currentMapInstance = 0;
}

WorldEntity::~WorldEntity()
{
	if (m_animLoader)
	{
		delete m_animLoader;
		m_animLoader = nullptr;
	}
}

void WorldEntity::SetBasePosition(float x, float y)
{
	m_baseX = x - (GetWidth() * 0.5f);
	m_baseY = y - GetHeight();
	m_worldX = m_baseX;
	m_worldY = m_baseY;
}

void WorldEntity::Update(float _deltaTime, float _cameraX, int _screenWidth, int _mapPixelWidth)
{
	// Check if entity went behind camera - reposition ahead
	float cameraLeftEdge = _cameraX;
	float entityRightEdge = m_worldX + GetWidth();
	
	if (entityRightEdge < cameraLeftEdge - 50.0f)
	{
		RepositionAhead(_cameraX, _screenWidth, _mapPixelWidth);
	}
}

void WorldEntity::Render(Renderer* _renderer, Camera* _camera)
{
	if (!m_isActive)
		return;
	
	float width = GetWidth();
	float height = GetHeight();
	
	// Convert world position to screen position
	float screenX = _camera ? _camera->WorldToScreenX(m_worldX) : m_worldX;
	float screenY = m_worldY;
	
	// Only render if on screen
	Point screenSize = _renderer->GetWindowSize();
	if (screenX < -width || screenX > screenSize.X + width)
		return;
	
	Rect destRect(
		(unsigned)screenX,
		(unsigned)screenY,
		(unsigned)(screenX + width),
		(unsigned)(screenY + height));
	
	string currentAnim = GetCurrentAnimationName();
	
	Rect srcRect = m_animLoader->UpdateAnimation(currentAnim, Timing::Instance().GetDeltaTime());
	Texture* currentTexture = m_animLoader->GetTexture(currentAnim);
	
	if (currentTexture)
		_renderer->RenderTexture(currentTexture, srcRect, destRect);
}

void WorldEntity::Reset()
{
	m_worldX = m_baseX;
	m_worldY = m_baseY;
	m_currentMapInstance = 0;
	m_isActive = true;
}

void WorldEntity::GetCollisionBox(float& outX, float& outY, float& outWidth, float& outHeight) const
{
	outX = m_worldX;
	outY = m_worldY;
	outWidth = GetWidth();
	outHeight = GetHeight();
}

void WorldEntity::RepositionAhead(float _cameraX, int _screenWidth, int _mapPixelWidth)
{
	// Calculate which map instance is ahead of camera
	float aheadX = _cameraX + _screenWidth + 100.0f;
	int targetMapInstance = (int)floor(aheadX / _mapPixelWidth);
	
	if (targetMapInstance <= m_currentMapInstance)
		targetMapInstance = m_currentMapInstance + 1;
	
	m_currentMapInstance = targetMapInstance;
	
	// Reset position to base position in new map instance
	float mapOffset = m_currentMapInstance * _mapPixelWidth;
	m_worldX = m_baseX + mapOffset;
	m_worldY = m_baseY;
	
	m_isActive = true;
	
	// Allow derived classes to customize behavior
	OnRepositionAhead();
}
