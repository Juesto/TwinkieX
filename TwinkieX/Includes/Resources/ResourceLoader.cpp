#include "pch.h"
#include "ResourceLoader.h"

ResourceLoader::ResourceLoader(int Id, const wchar_t* Type)
{
	Load(Id, Type);
}

ResourceLoader::ResourceLoader() {}

static HMODULE GetCurrentModule()
{
	HMODULE hModule = NULL;
	GetModuleHandleExW(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		reinterpret_cast<LPCWSTR>(&GetCurrentModule),
		&hModule
	);
	return hModule;
}

void ResourceLoader::Load(int Id, const wchar_t* Type)
{
	auto TwinkieXModule = GetCurrentModule();
	auto Resource = FindResource(TwinkieXModule, MAKEINTRESOURCE(Id), Type);
	if (!Resource) return;

	auto ResourceHandle = LoadResource(TwinkieXModule, Resource);
	if (!ResourceHandle or ResourceHandle == INVALID_HANDLE_VALUE) return;

	auto ResourceData = (uint8_t*)LockResource(ResourceHandle);
	auto ResourceSize = SizeofResource(TwinkieXModule, Resource);

	Data.assign(ResourceData, ResourceData + ResourceSize);

	FreeResource(ResourceHandle);
}