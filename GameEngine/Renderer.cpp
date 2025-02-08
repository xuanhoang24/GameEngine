#include "Renderer.h"
#include "Asset.h"

Renderer::Renderer()
{
    m_window = nullptr;
    m_renderer = nullptr;
    m_destRect = { };
    m_surface = nullptr;
}

Renderer::~Renderer()
{
    Shutdown();
}

void Renderer::Initialize(int _xResolution, int _yResolution)
{
    M_ASSERT((SDL_Init(SDL_INIT_EVERYTHING) >= 0), "SDL initialization failed.");
    m_window = SDL_CreateWindow("SDL Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _xResolution, _yResolution, SDL_WINDOW_SHOWN);
    M_ASSERT(m_window != nullptr, "Failed to initialize SDL window.");
    m_renderer = SDL_CreateRenderer(m_window, -1, 0);
    M_ASSERT(m_renderer != nullptr, "Failed to initialize SDL renderer.");
}

void Renderer::Shutdown()
{
    for (auto it = m_textures.begin(); it != m_textures.end(); it++)
    {
        SDL_DestroyTexture(it->second);
    }
    m_textures.clear();
    if (m_renderer != nullptr)
    {
        SDL_DestroyRenderer(m_renderer);
    }
    if (m_window != nullptr)
    {
        SDL_DestroyWindow(m_window);
    }
    SDL_Quit(); // Quit SDL subsystems
}

void Renderer::SetDrawColor(Color _color)
{
    SDL_SetRenderDrawColor(m_renderer, _color.R, _color.G, _color.B, _color.A);
}

void Renderer::ClearScreen()
{
    SDL_RenderClear(m_renderer);
}

void Renderer::RenderPoint(Point _position)
{
    SDL_RenderDrawPoint(m_renderer, _position.X, _position.Y);
}

void Renderer::RenderLine(Rect _points)
{
    SDL_RenderDrawLine(m_renderer, _points.X1, _points.Y1, _points.X2, _points.Y2);
}

void Renderer::RenderRectangle(Rect _rect)
{
    m_destRect.x = _rect.X1;
    m_destRect.y = _rect.Y1;
    m_destRect.w = _rect.X2 - _rect.X1;
    m_destRect.h = _rect.Y2 - _rect.Y1;
    SDL_RenderDrawRect(m_renderer, &m_destRect);
}

void Renderer::RenderFillRectangle(Rect _rect)
{
    m_destRect.x = _rect.X1;
    m_destRect.y = _rect.Y1;
    m_destRect.w = _rect.X2 - _rect.X1;
    m_destRect.h = _rect.Y2 - _rect.Y1;
    SDL_RenderFillRect(m_renderer, &m_destRect);
}

SDL_Texture* Renderer::GetSDLTexture(Texture* _texture)
{
    // If texture has already been created, return created texture
    Asset* asset = _texture->GetData();
    string guid = asset->GetGUID();
    if (m_textures.count(guid) != 0)
    {
        return m_textures[guid];
    }

    // Otherwise, create the GPU texture
    ImageInfo* ii = _texture->GetImageInfo();
    m_surface = SDL_CreateRGBSurfaceFrom(asset->GetData() + _texture->GetImageInfo()->DataOffset, ii->Width, ii->Height, ii->BitsPerPixel,
        ii->Width * ii->BitsPerPixel / 8, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, m_surface);
    SDL_FreeSurface(m_surface);
    m_surface = nullptr;
    m_textures[guid] = texture;
    return texture;
}

void Renderer::RenderTexture(Texture* _texture, Point _point)
{
    m_destRect.x = _point.X;
    m_destRect.y = _point.Y;
    m_destRect.w = _texture->GetImageInfo()->Width;
    m_destRect.h = _texture->GetImageInfo()->Height;
    M_ASSERT(((SDL_RenderCopyEx(m_renderer, GetSDLTexture(_texture), NULL, &m_destRect, 0, NULL, SDL_FLIP_VERTICAL)) >= 0), "Could not render texture");
}
