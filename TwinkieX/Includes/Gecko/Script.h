#pragma once
#include "pch.h"
#include "CtxSet.h"

namespace Gecko
{
	class Script
	{
	public:
		Script operator=(Script) = delete;

		Script(const Script&) = default;
		Script(Script&&) = default;
		Script(Script&) = default;
		Script() = default;

		std::filesystem::path ScriptPath = "";
		std::u8string Id = u8"";
		std::u8string Name = u8"";
		std::u8string SourceCode = u8"";
		/* TODO: Logging */
		CtxSet Ctxs{};
		size_t YieldFor = 0;
		bool Enabled = true;
	};
}