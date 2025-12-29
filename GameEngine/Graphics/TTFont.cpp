#include "../Graphics/TTFont.h"
#include "../Graphics/Renderer.h"

TTFont::TTFont()
{
	m_font = nullptr;
	destRect = { };
}
TTFont::~TTFont()
{
	Shutdown();
}
void TTFont::Initialize(int _pointSize)
{
	if (!TTF_WasInit())
	{
		M_ASSERT((TTF_Init() >= 0), "Unable to initialize SDL TTF.");
	}
	M_ASSERT((m_font = TTF_OpenFont("../Assets/Fonts/arial.ttf", _pointSize)) != nullptr, "Failed to load font");
}

void TTFont::Shutdown()
{
	TTF_Quit;
}

void TTFont::Write(SDL_Renderer* _renderer, const char* _text, SDL_Color _color, SDL_Point _pos)
{
	if (strlen(_text) == 0) return;

	SDL_Surface* surface;
	surface = TTF_RenderUTF8_Blended(m_font, _text, _color);

	SDL_Texture* texture;
	texture = SDL_CreateTextureFromSurface(_renderer, surface);

	SDL_Rect destRect{ _pos.x, _pos.y, surface->w,surface->h };
	M_ASSERT(((SDL_RenderCopyEx(_renderer, texture, nullptr, &destRect, 0, nullptr, SDL_FLIP_NONE)) >= 0), "Could not render texture");
	
	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);
}

void TTFont::Write(SDL_Renderer* _renderer, int _fontSize, const char* _text, SDL_Color _color, SDL_Point _pos, int* _outWidth, int* _outHeight)
{
	if (strlen(_text) == 0) return;

	TTF_Font* tempFont = TTF_OpenFont("../Assets/Fonts/arial.ttf", _fontSize);
	if (!tempFont) return;

	SDL_Surface* surface = TTF_RenderUTF8_Blended(tempFont, _text, _color);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(_renderer, surface);

	SDL_Rect destRect{ _pos.x, _pos.y, surface->w, surface->h };
	SDL_RenderCopyEx(_renderer, texture, nullptr, &destRect, 0, nullptr, SDL_FLIP_NONE);

	if (_outWidth) *_outWidth = surface->w;
	if (_outHeight) *_outHeight = surface->h;

	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);
	TTF_CloseFont(tempFont);
}

void TTFont::GetTextSize(const char* _text, int* _width, int* _height)
{
	if (m_font && strlen(_text) > 0)
	{
		TTF_SizeText(m_font, _text, _width, _height);
	}
	else
	{
		*_width = 0;
		*_height = 0;
	}
}
