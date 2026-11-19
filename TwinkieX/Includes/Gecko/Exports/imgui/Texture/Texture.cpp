#include "pch.h"
#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <Twinkie/TwinkUi/TwinkUi.h>

#ifdef X64
// Functions from https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples#example-for-directx11-users

// Simple helper function to load an image into a DX11 texture with common settings
static bool LoadTextureFromMemory(const void* data, size_t data_size, TextureResource* out_texture, int* out_width, int* out_height)
{
    // Load from disk into a raw RGBA buffer
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load_from_memory((const unsigned char*)data, (int)data_size, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create texture
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = image_width;
    desc.Height = image_height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;

    ID3D11Texture2D* pTexture = NULL;
    D3D11_SUBRESOURCE_DATA subResource;
    subResource.pSysMem = image_data;
    subResource.SysMemPitch = desc.Width * 4;
    subResource.SysMemSlicePitch = 0;
    TwinkUiState::Device->CreateTexture2D(&desc, &subResource, &pTexture);
    if (!pTexture) return false;

    // Create texture view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    TwinkUiState::Device->CreateShaderResourceView(pTexture, &srvDesc, out_texture);
    pTexture->Release();

    *out_width = image_width;
    *out_height = image_height;
    stbi_image_free(image_data);

    return true;
}


// Open and read a file, then forward to LoadTextureFromMemory()
bool LoadTextureFromFile(const char* file_name, TextureResource* out_srv, int* out_width, int* out_height)
{
    FILE* f; 
    fopen_s(&f, file_name, "rb");
    if (f == NULL)
        return false;
    fseek(f, 0, SEEK_END);
    size_t file_size = (size_t)ftell(f);
    if (file_size == (size_t)-1)
        return false;
    fseek(f, 0, SEEK_SET);
    void* file_data = IM_ALLOC(file_size);
    fread(file_data, 1, file_size, f);
    fclose(f);
    bool ret = LoadTextureFromMemory(file_data, file_size, out_srv, out_width, out_height);
    IM_FREE(file_data);
    return ret;
}
#else
// Functions from https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples#example-for-directx9-users

// Modified to load from memory.
static bool LoadTextureFromMemory(const void* data, size_t data_size, TextureResource* out_texture, int* out_width, int* out_height)
{
    // Load texture from disk
    PDIRECT3DTEXTURE9 texture;
    HRESULT hr = D3DXCreateTextureFromFileInMemory(TwinkUiState::Device, data, data_size, &texture);
    if (hr != S_OK)
        return false;

    // Retrieve description of the texture surface so we can access its size
    D3DSURFACE_DESC my_image_desc;
    texture->GetLevelDesc(0, &my_image_desc);
    *out_texture = texture;
    *out_width = (int)my_image_desc.Width;
    *out_height = (int)my_image_desc.Height;
    return true;
}

// Simple helper function to load an image into a DX9 texture with common settings
static bool LoadTextureFromFile(const char* filename, TextureResource* out_texture, int* out_width, int* out_height)
{
    // Load texture from disk
    PDIRECT3DTEXTURE9 texture;
    HRESULT hr = D3DXCreateTextureFromFileA(TwinkUiState::Device, filename, &texture);
    if (hr != S_OK)
        return false;

    // Retrieve description of the texture surface so we can access its size
    D3DSURFACE_DESC my_image_desc;
    texture->GetLevelDesc(0, &my_image_desc);
    *out_texture = texture;
    *out_width = (int)my_image_desc.Width;
    *out_height = (int)my_image_desc.Height;
    return true;
}
#endif

Texture::Texture() {}
Texture::Texture(void* Memory, size_t Size) { From(Memory, Size); }
Texture::Texture(const std::filesystem::path& Path) { From(Path); }

Texture::~Texture()
{
    InternalTexture->Release();
}

bool Texture::From(void* Memory, size_t Size)
{
    return LoadTextureFromMemory(Memory, Size, &InternalTexture, (int*)&Width, (int*)&Height);
}

bool Texture::From(const std::filesystem::path& Path)
{
    return LoadTextureFromFile(Path.string().c_str(), &InternalTexture, (int*)&Width, (int*)&Height);
}

ImTextureID Texture::Get() const
{
    return (ImTextureID)(intptr_t)InternalTexture;
}

ImVec2 Texture::GetSize() const
{
    return ImVec2(float(Width), float(Height));
}

void Texture::AddRef()
{
    RefCount++;
}

void Texture::RemoveRef()
{
    if (!--RefCount) delete this;
}