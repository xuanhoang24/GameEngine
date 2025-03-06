#include "Renderer.h"
#include "Asset.h"

Renderer::Renderer()
{
    m_window = nullptr;
    m_renderer = nullptr;
    m_srcRect = { };
    m_destRect = { };
    m_surface = nullptr;
    m_viewPort = { };
}

Renderer::~Renderer()
{
    Shutdown();
}

void Renderer::Initialize()
{
    M_ASSERT((SDL_Init(SDL_INIT_EVERYTHING) >= 0), "SDL initialization failed.");
    SDL_GetDisplayBounds(0, &m_srcRect);
    m_window = SDL_CreateWindow("SDL Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        m_srcRect.w, m_srcRect.y, SDL_WINDOW_FULLSCREEN);
    M_ASSERT(m_window != nullptr, "Failed to initialize SDL window.");
    m_renderer = SDL_CreateRenderer(m_window, -1, 0);
    M_ASSERT(m_renderer != nullptr, "Failed to initialize SDL renderer.");
}

void Renderer::EnumerateDisplayModes()
{
    int display_count = SDL_GetNumVideoDisplays();
    for (int display_index = 0; display_index <= display_count; display_index++)
    {
        int modes_count = SDL_GetNumDisplayModes(display_index);
        for (int mode_index = 0; mode_index <= modes_count; mode_index++)
        {
            SDL_DisplayMode mode = { SDL_PIXELFORMAT_UNKNOWN,0,0,0,0 };
            if (SDL_GetDisplayMode(display_index, mode_index, &mode) == 0)
            {
                m_resolutions.push_back(mode);
            }
        }
    }
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
    if (m_textures.count(guid) == 0)
    {
        // If not found create the GPU texture
        ImageInfo* ii = _texture->GetImageInfo();
        m_surface = SDL_CreateRGBSurfaceFrom(asset->GetData() + _texture->GetImageInfo()->DataOffset, ii->Width, ii->Height, ii->BitsPerPixel,
            ii->Width * ii->BitsPerPixel / 8, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, m_surface);
        SDL_FreeSurface(m_surface);
        m_surface = nullptr;
        m_textures[guid] = texture;
    }

    SDL_SetTextureBlendMode(m_textures[guid], _texture->GetBlendMode());
    SDL_SetTextureAlphaMod(m_textures[guid], _texture->GetBlendAlpha());
    return m_textures[guid];
}

void Renderer::RenderTexture(Texture* _texture, Point _point)
{
    m_destRect.x = _point.X;
    m_destRect.y = _point.Y;
    m_destRect.w = _texture->GetImageInfo()->Width;
    m_destRect.h = _texture->GetImageInfo()->Height;
    M_ASSERT(((SDL_RenderCopyEx(m_renderer, GetSDLTexture(_texture), NULL, &m_destRect, 0, NULL, SDL_FLIP_VERTICAL)) >= 0), "Could not render texture");
}

Point Renderer::GetWindowSize() 
{
    int w;
    int h;
    SDL_GetWindowSize(m_window, &w, &h);
    return Point(w, h);
}

void Renderer::SetViewport(Rect _viewport)
{
    m_viewPort.x = _viewport.X1;
    m_viewPort.y = _viewport.Y1;
    m_viewPort.w = _viewport.X2 - _viewport.X1;
    m_viewPort.h = _viewport.Y2 - _viewport.Y1;
    SDL_RenderSetViewport(m_renderer, &m_viewPort);
}

void Renderer::RenderTexture(Texture* _texture, Rect _rect)
{
    m_destRect.x = _rect.X1;
    m_destRect.y = _rect.Y1;
    m_destRect.w = _rect.X2 - _rect.X1;
    m_destRect.h = _rect.Y2 - _rect.Y1;
    M_ASSERT(((SDL_RenderCopyEx(m_renderer, GetSDLTexture(_texture),
        NULL, &m_destRect, 0, NULL, SDL_FLIP_VERTICAL)) >= 0), "Could not render texture");
}

void Renderer::RenderTexture(Texture* _texture, Rect _srcRect, Rect _destRect)
{
    m_destRect.x = _destRect.X1;
    m_destRect.y = _destRect.Y1;
    m_destRect.w = _destRect.X2 - _destRect.X1;
    m_destRect.h = _destRect.Y2 - _destRect.Y1;

    m_srcRect.x = _srcRect.X1;
    m_srcRect.y = _texture->GetImageInfo()->Height - _srcRect.Y2;
    m_srcRect.w = _srcRect.X2 - _srcRect.X1;
    m_srcRect.h = _srcRect.Y2 - _srcRect.Y1;

    M_ASSERT(((SDL_RenderCopyEx(m_renderer, GetSDLTexture(_texture),
        &m_srcRect, &m_destRect, 0, NULL, SDL_FLIP_VERTICAL)) >= 0), "Could not render texture");
}

void Renderer::RenderTexture(SDL_Texture* _texture, Rect _srcRect, Rect _destRect, double _angle)
{
    SDL_Point size;
    SDL_QueryTexture(_texture, NULL, NULL, &size.x, &size.y);

    m_destRect.x = _destRect.X1;
    m_destRect.y = _destRect.Y1;
    m_destRect.w = _destRect.X2 - _destRect.X1;
    m_destRect.h = _destRect.Y2 - _destRect.Y1;

    m_srcRect.x = _srcRect.X1;
    m_srcRect.y = _srcRect.Y1;
    m_srcRect.w = _srcRect.X2 - _srcRect.X1;
    m_srcRect.h = _srcRect.Y2 - _srcRect.Y1;

    M_ASSERT(((SDL_RenderCopyEx(m_renderer, _texture, &m_srcRect, &m_destRect, _angle, 
        nullptr, SDL_FLIP_NONE)) >= 0), "Could not render texture");
}
