#pragma once
#include "resource.h"
#include "pch.h"

class ResourceLoader
{
public:
	std::vector<uint8_t> Data;

	ResourceLoader(int Id, const wchar_t* Type);
	ResourceLoader();

	void Load(int Id, const wchar_t* Type);
};