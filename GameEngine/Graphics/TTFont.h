#ifndef TTFONT_H
#define TTFONT_H

#include "../Core/StandardIncludes.h"

class Renderer;

class TTFont
{
public:
	//Constructors/ Destructors
	TTFont();
	virtual ~TTFont();

	//Methods
	void Initialize(int _pointSize);
	void Write(SDL_Renderer* _renderer, const char* _text, SDL_Color, SDL_Point _pos);
	void Write(SDL_Renderer* _renderer, int _fontSize, const char* _text, SDL_Color, SDL_Point _pos, int* _outWidth = nullptr, int* _outHeight = nullptr);
	void GetTextSize(const char* _text, int* _width, int* _height);
	void Shutdown();

private:
	//Methods
	TTF_Font* m_font;
	SDL_Rect destRect;
};

#endif // TTFONT_H

