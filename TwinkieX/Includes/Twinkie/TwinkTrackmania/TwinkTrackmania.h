#pragma once

// Offsets for various things
#include <Utils/Offsets.h>

// Various types used by Trackmania
#include <Trackmania/TMTypes.h>

// Useful for realloc
#include <cstdlib>

#include <type_traits>
#include <cassert>
#include <vector>
#include <string>

// The Trackmania manager class, used by (TwinkTrackmania gTwinkie.TrackmaniaMgr).
class TwinkTrackmania
{
public:
	template <typename ReturnT>
	using GetterReturnT = std::conditional_t<std::is_same_v<ReturnT, CMwNod*>, CMwNod*, ReturnT*>;

	uintptr_t ExeBaseAddr = 0;

	// Behaviors

	// Ctor
	__declspec(noinline) TwinkTrackmania();
	// Dtor
	__declspec(noinline) virtual ~TwinkTrackmania();


	// Getters

	// Scans for a specific signature (sigscan) in the .exe.
	// The signature MUST have at least 4 bytes, all non-wildcard (i.e. not ?).
	uintptr_t SigScan(const char* Sig);

	// Gets the main app object, usually defined as (CTrackMania*).
	__declspec(noinline) CMwNod* GetApp();

	// Gets the address to the Viewport of the main app
	__declspec(noinline) CMwNod* GetViewport();

	// Gets the game's DirectX device, as LPDIRECT3DDEVICE9 for GAMEBOX, and ID3D11Device for MANIAPLANET
	__declspec(noinline) uintptr_t GetDirectXDevice();

	// Gets the game's input port, for mouse and keyboard events (always CDx8InputPort or similar)
	__declspec(noinline) CInputPort* GetInputPort();

#ifdef GAMEBOX
	// Sees if the intro when launching the game is over or not. Only for GAMEBOX
	__declspec(noinline) bool GetIsIntroOver();
#endif

	// Not for direct use. Use ParamGet instead.
	template<typename ReturnT>
	GetterReturnT<typename ReturnT> VirtualParamGet(CMwNod* Nod, CMwMemberInfo* MemberInfo);

	// Not for direct use. Use ParamGet instead.
	template<typename ReturnT>
	GetterReturnT<typename ReturnT> VirtualParamGet(CMwNod* Nod, const char* MemberName);

	// Not for direct use. Use ParamSet instead.
	template<typename ValueT>
	bool VirtualParamSet(CMwNod* Nod, CMwMemberInfo* MemberInfo, ValueT* Value);

	// Not for direct use. Use ParamSet instead.
	template<typename ValueT>
	bool VirtualParamSet(CMwNod* Nod, const char* MemberName, ValueT* Value);

	// Gets a member's value from a nod. If return result is null, then something failed. Returns pointer to requested datatype unless datatype is CMwNod*, then it returns CMwNod*.
	template<typename ReturnT>
	GetterReturnT<typename ReturnT> ParamGet(CMwNod* Nod, CMwMemberInfo* MemberInfo, bool IsNodImpersistent = false);

	// Gets a member's value from a nod. If return result is null, then something failed. Returns pointer to requested datatype unless datatype is CMwNod*, then it returns CMwNod*.
	template<typename ReturnT>
	GetterReturnT<typename ReturnT> ParamGet(CMwNod* Nod, const char* MemberName, bool IsNodImpersistent = false);

	// Sets a member to a value for a nod. Returns true if succeeded.
	template<typename ValueT>
	bool ParamSet(CMwNod* Nod, CMwMemberInfo* MemberInfo, ValueT* Value, bool IsNodImpersistent = false);

	// Sets a member to a value for a nod. Returns true if succeeded.
	template<typename ValueT>
	bool ParamSet(CMwNod* Nod, const char* MemberName, ValueT* Value, bool IsNodImpersistent = false);

	// Pushes member to stack.
	void PushToStack(CMwStack* Stack, CMwMemberInfo* MemberInfo);

#ifdef MANIAPLANET
	// Gets the game's DirectX swap chain, only for TMCN/D3D11
	__declspec(noinline) uintptr_t GetDirectXSwapChain();
#endif
};

template<typename ReturnT>
TwinkTrackmania::GetterReturnT<typename ReturnT> TwinkTrackmania::VirtualParamGet(CMwNod* Nod, CMwMemberInfo* MemberInfo)
{
	// This variable is used only to not let the compiler optimize the type of Stack->ppMemberInfos for GAMEBOX to a single pointer.
	static CMwStack* PreviousUsedStack = nullptr;

	struct
	{
		GetterReturnT<ReturnT> pReturnValue = nullptr;
		uint8_t Storage[sizeof(ReturnT)] = {};
	} Result;

	CMwStack Stack = CMwStack();

	PushToStack(&Stack, MemberInfo);

	PreviousUsedStack = &Stack;

	Nod->VirtualParam_Get(&Stack, &Result);

	return Result.pReturnValue;
}

template<typename ReturnT>
TwinkTrackmania::GetterReturnT<typename ReturnT> TwinkTrackmania::VirtualParamGet(CMwNod* Nod, const char* MemberName)
{
	// This variable is used only to not let the compiler optimize the type of Stack->ppMemberInfos for GAMEBOX to a single pointer.
	static CMwStack* PreviousUsedStack = nullptr;

	auto ClassInfo = Nod->MwGetClassInfo();
	CMwMemberInfo* MemberInfo = nullptr;

	while (ClassInfo)
	{ 
		for (auto& Member : *ClassInfo)
		{
			if (strcmp(Member->MemberName, MemberName) == 0)
			{
				MemberInfo = Member;
				break;
			}
		}
		ClassInfo = ClassInfo->ParentClassInfo;
	}

	if (!MemberInfo)
	{
		return nullptr;
	}

	using ReturnTypeT = std::conditional_t<std::is_same_v<ReturnT, CMwNod*>, CMwNod*, ReturnT*>;

	struct
	{
		ReturnTypeT pReturnValue;
		ReturnT Storage;
	} Result;

	CMwStack Stack = CMwStack();
	PushToStack(&Stack, MemberInfo);

	PreviousUsedStack = &Stack;

	Nod->VirtualParam_Get(&Stack, &Result);

	return Result.pReturnValue;
}

template<typename ValueT>
bool TwinkTrackmania::VirtualParamSet(CMwNod* Nod, CMwMemberInfo* MemberInfo, ValueT* Value)
{
	CMwStack Stack = CMwStack();
	PushToStack(&Stack, MemberInfo);

	return !Nod->VirtualParam_Set(&Stack, Value);
}

template<typename ValueT>
bool TwinkTrackmania::VirtualParamSet(CMwNod* Nod, const char* MemberName, ValueT* Value)
{
	auto ClassInfo = Nod->MwGetClassInfo();
	CMwMemberInfo* MemberInfo = nullptr;

	while (ClassInfo)
	{
		for (auto& Member : *ClassInfo)
		{
			if (strcmp(Member->MemberName, MemberName) == 0)
			{
				MemberInfo = Member;
				break;
			}
		}
		ClassInfo = ClassInfo->ParentClassInfo;
	}

	if (!MemberInfo)
	{
		return false;
	}

	CMwStack Stack = CMwStack();
	PushToStack(&Stack, MemberInfo);

	return !Nod->VirtualParam_Set(&Stack, Value);
}

template<typename ReturnT>
TwinkTrackmania::GetterReturnT<typename ReturnT> TwinkTrackmania::ParamGet(CMwNod* Nod, CMwMemberInfo* MemberInfo, bool IsNodImpersistent)
{
	if (MemberInfo->MemberOffset == 0xFFFFFFFFU and not IsNodImpersistent)
	{
		return VirtualParamGet<ReturnT>(Nod, MemberInfo);
	}
	if constexpr (not std::is_same_v<ReturnT, CMwNod*>)
	{
		return &ReadAddr(ReturnT, ((uintptr_t)Nod) + MemberInfo->MemberOffset);
	}
	else
	{
		return ReadAddr(ReturnT, ((uintptr_t)Nod) + MemberInfo->MemberOffset);
	}
}

template<typename ReturnT>
TwinkTrackmania::GetterReturnT<typename ReturnT> TwinkTrackmania::ParamGet(CMwNod* Nod, const char* MemberName, bool IsNodImpersistent)
{
	auto ClassInfo = Nod->MwGetClassInfo();
	CMwMemberInfo* MemberInfo = nullptr;

	while (ClassInfo)
	{
		for (auto& Member : *ClassInfo)
		{
			if (strcmp(Member->MemberName, MemberName) == 0)
			{
				MemberInfo = Member;
				break;
			}
		}
		ClassInfo = ClassInfo->ParentClassInfo;
	}

	if (!MemberInfo)
	{
		return nullptr;
	}

	if (MemberInfo->MemberOffset == 0xFFFFFFFFU and not IsNodImpersistent)
	{
		if constexpr (std::is_same_v<CMwNod*, ReturnT>)
		{
			return reinterpret_cast<CMwNod*>(VirtualParamGet<CMwNod*>(Nod, MemberInfo));
		}
		else 
		{
			return VirtualParamGet<ReturnT>(Nod, MemberInfo);
		}
	}
	if constexpr (not std::is_same_v<ReturnT, CMwNod*>)
	{
		return &ReadAddr(ReturnT, ((uintptr_t)Nod) + MemberInfo->MemberOffset);
	}
	else
	{
		return ReadAddr(ReturnT, ((uintptr_t)Nod) + MemberInfo->MemberOffset);
	}
}

template<typename ValueT>
bool TwinkTrackmania::ParamSet(CMwNod* Nod, CMwMemberInfo* MemberInfo, ValueT* Value, bool IsNodImpersistent)
{
	if (MemberInfo->MemberOffset == 0xFFFFFFFFU and not IsNodImpersistent)
	{
		return VirtualParamSet(Nod, MemberInfo, Value);
	}
	WriteAddr(ValueT, (uintptr_t)Nod + MemberInfo->MemberOffset, *Value);
	return true;
}

template<typename ValueT>
bool TwinkTrackmania::ParamSet(CMwNod* Nod, const char* MemberName, ValueT* Value, bool IsNodImpersistent)
{
	auto ClassInfo = Nod->MwGetClassInfo();
	CMwMemberInfo* MemberInfo = nullptr;

	while (ClassInfo)
	{
		for (auto& Member : *ClassInfo)
		{
			if (strcmp(Member->MemberName, MemberName) == 0)
			{
				MemberInfo = Member;
				break;
			}
		}
		ClassInfo = ClassInfo->ParentClassInfo;
	}

	if (!MemberInfo)
	{
		return false;
	}

	if (MemberInfo->MemberOffset == 0xFFFFFFFFU and not IsNodImpersistent)
	{
		return VirtualParamSet(Nod, MemberInfo, Value);
	}
	WriteAddr(ValueT, (uintptr_t)Nod + MemberInfo, *Value);
	return true;
}