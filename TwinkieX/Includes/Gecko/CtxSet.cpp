#include "pch.h"
#include "CtxSet.h"

namespace Gecko
{
	void CtxSet::ReleaseAll()
	{
		for (size_t Idx = 0; Idx < sizeof(CtxSet) / sizeof(uintptr_t); Idx++)
		{
			asIScriptContext* Ctx = ((asIScriptContext**)this)[Idx];
			if (Ctx) Ctx->Release();
		}
	}

	asIScriptContext*& CtxSet::operator[](size_t Idx)
	{
		return ((asIScriptContext**)this)[Idx];
	}

	const asIScriptContext*& CtxSet::operator[](const size_t Idx) const
	{
		return ((const asIScriptContext**)this)[Idx];
	}
}