#pragma once
#include "pch.h"

namespace Gecko::Exports
{
	struct GeckoAutoExport;
	namespace ImGui
	{
		void Registrar(asIScriptEngine*);
	}
}

#ifdef X64
using TextureResource = ID3D11ShaderResourceView*;
#else
using TextureResource = IDirect3DTexture9*;
#endif

class Texture
{
	TextureResource InternalTexture = nullptr;
	size_t Width = 0, Height = 0;
	mutable size_t RefCount = 0;

	friend void Gecko::Exports::ImGui::Registrar(asIScriptEngine*);

	void AddRef();
	void RemoveRef();
public:
	Texture();
	Texture(void* Memory, size_t Size);
	Texture(const std::filesystem::path& Path);
	~Texture();

	bool From(void* Memory, size_t Size);
	bool From(const std::filesystem::path& Path);
	ImTextureID Get() const;
	ImVec2 GetSize() const;
};