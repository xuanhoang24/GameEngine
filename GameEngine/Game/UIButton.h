#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include "../Core/StandardIncludes.h"

class Renderer;
class TTFont;

// Helper class for UI button handling
class UIButton
{
public:
	UIButton();
	
	void SetRect(int x, int y, int width, int height);
	void SetRect(SDL_Rect rect) { m_rect = rect; }
	SDL_Rect& GetRect() { return m_rect; }
	
	bool IsHovered() const { return m_hovered; }
	bool IsClicked() const { return m_clicked; }
	
	// Update hover state based on mouse position
	void UpdateHover(int mouseX, int mouseY);
	
	// Check if button was clicked (call after UpdateHover)
	bool CheckClick(SDL_Event& event);
	
	// Render the button
	void Render(Renderer* _renderer, TTFont* _font, const char* _text, bool _isExit = false);
	
	// Reset state
	void Reset();

private:
	SDL_Rect m_rect;
	bool m_hovered;
	bool m_clicked;
};

#endif // UI_BUTTON_H
