#include "../Graphics/PNGReader.h"
#include "../Resources/AssetController.h"

PNGReader::PNGReader()
{
    m_data = nullptr;
}

PNGReader::~PNGReader()
{
}

void PNGReader::ProcessAsset(Asset* _rawPNG, ImageInfo* _imageInfo)
{
    // Decode PNG from memory
    SDL_RWops* rw = SDL_RWFromConstMem(_rawPNG->GetData(), _rawPNG->GetDataSize());
    M_ASSERT(rw != nullptr, "SDL_RWFromConstMem failed");

    SDL_Surface* surface = IMG_Load_RW(rw, 0);
    M_ASSERT(surface != nullptr, "IMG_Load_RW failed (invalid PNG)");

    // Convert to 32-bit BGRA to match renderer's BGR byte order
    SDL_Surface* conv = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_BGRA32, 0);
    SDL_FreeSurface(surface);
    M_ASSERT(conv != nullptr, "SDL_ConvertSurfaceFormat failed");

    // Flip vertically to match TGA format
    int pitch = conv->pitch;
    int height = conv->h;
    unsigned char* pixels = (unsigned char*)conv->pixels;
    unsigned char* temp = new unsigned char[pitch];
    
    for (int i = 0; i < height / 2; i++)
    {
        unsigned char* row1 = pixels + i * pitch;
        unsigned char* row2 = pixels + (height - i - 1) * pitch;
        memcpy(temp, row1, pitch);
        memcpy(row1, row2, pitch);
        memcpy(row2, temp, pitch);
    }
    delete[] temp;

    _imageInfo->Width = conv->w;
    _imageInfo->Height = conv->h;
    _imageInfo->BitsPerPixel = 32;
    _imageInfo->DataOffset = 0;

    // The Asset buffer now contain DECODED RGBA pixels
    int newSize = conv->w * conv->h * 4;

    // Replace asset buffer (stack allocator)
    _rawPNG->SetDataSize(newSize);
    _rawPNG->SetData(AssetController::Stack->GetMemory(newSize));

    // Copy pixel data into asset buffer
    memcpy(_rawPNG->GetData(), conv->pixels, newSize);

    SDL_FreeSurface(conv);
    SDL_RWclose(rw);
}

Asset* PNGReader::LoadPNGFromFile(string _file, ImageInfo* _imageInfo)
{
    //Read the file into an asset
    m_data = AssetController::Instance().GetAsset(_file);
    ProcessAsset(m_data, _imageInfo);
    return m_data;
}