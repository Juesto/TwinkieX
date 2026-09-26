#pragma once
#include "pch.h"
#include "CtxSet.h"

namespace Gecko
{
	class Script
	{
	public:
		std::filesystem::path ScriptPath;
		std::u8string Id;
		std::u8string Name;
		std::u8string SourceCode;
		CtxSet Ctxs;
		size_t YieldFor = 0;
		bool Enabled = true;
	};
}