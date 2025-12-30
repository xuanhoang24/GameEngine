#include "../Game/UIButton.h"
#include "../Graphics/Renderer.h"
#include "../Graphics/TTFont.h"

UIButton::UIButton()
{
	m_rect = { 0, 0, 0, 0 };
	m_hovered = false;
	m_clicked = false;
}

void UIButton::SetRect(int x, int y, int width, int height)
{
	m_rect = { x, y, width, height };
}

void UIButton::UpdateHover(int mouseX, int mouseY)
{
	m_hovered = (mouseX >= m_rect.x && 
	             mouseX <= m_rect.x + m_rect.w &&
	             mouseY >= m_rect.y && 
	             mouseY <= m_rect.y + m_rect.h);
}

bool UIButton::CheckClick(SDL_Event& event)
{
	m_clicked = false;
	if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
	{
		if (m_hovered)
		{
			m_clicked = true;
		}
	}
	return m_clicked;
}

void UIButton::Render(Renderer* _renderer, TTFont* _font, const char* _text, bool _isExit)
{
	// Button background
	if (_isExit)
	{
		if (m_hovered)
			SDL_SetRenderDrawColor(_renderer->GetRenderer(), 180, 80, 80, 255);
		else
			SDL_SetRenderDrawColor(_renderer->GetRenderer(), 150, 50, 50, 255);
	}
	else
	{
		if (m_hovered)
			SDL_SetRenderDrawColor(_renderer->GetRenderer(), 80, 180, 80, 255);
		else
			SDL_SetRenderDrawColor(_renderer->GetRenderer(), 50, 150, 50, 255);
	}
	SDL_RenderFillRect(_renderer->GetRenderer(), &m_rect);
	
	// Button border
	SDL_SetRenderDrawColor(_renderer->GetRenderer(), 255, 255, 255, 255);
	SDL_RenderDrawRect(_renderer->GetRenderer(), &m_rect);
	
	// Button text centered
	int textW, textH;
	_font->GetTextSize(_text, &textW, &textH);
	SDL_Color white = { 255, 255, 255, 255 };
	SDL_Point textPos = { m_rect.x + (m_rect.w - textW) / 2, m_rect.y + (m_rect.h - textH) / 2 };
	_font->Write(_renderer->GetRenderer(), _text, white, textPos);
}

void UIButton::Reset()
{
	m_hovered = false;
	m_clicked = false;
}
