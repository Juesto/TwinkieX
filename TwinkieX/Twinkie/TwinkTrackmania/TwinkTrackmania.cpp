// Precompiled headers.
#include "pch.h"

#include <windows.h>

// Class definition for the TwinkTrackmania class, that also includes extra headers for Trackmania.
#include <Twinkie/TwinkTrackmania/TwinkTrackmania.h>

// Behaviors

TwinkTrackmania::TwinkTrackmania()
{
	this->ExeBaseAddr = (uintptr_t)GetModuleHandleA(NULL);

#ifdef TMCN
	// This is for removing the timelimit entirely.
	Unprotect(this->ExeBaseAddr + 0x84976c, 8);
	WriteAddr(uint64_t, this->ExeBaseAddr + 0x84976c, 0x3B'F0'8B'90'90'90'90'90);
#endif
}

TwinkTrackmania::~TwinkTrackmania()
{
}

// END Behaviors

// Getters

uintptr_t TwinkTrackmania::SigScan(const char* Sig)
{
	__pragma(pack(push, 1))
	struct OptionU8
	{
		bool IsWildcard;
		uint8_t Byte;
		
		bool operator==(uint8_t Rhs)
		{
			if (IsWildcard) return true;
			return Rhs == Byte;
		}
	};
	__pragma(pack(pop))

	std::vector<OptionU8> SigBytes;
	std::string SigStr = Sig;
	std::string Byte;

	for (auto& Char : SigStr)
	{
		if (Char == ' ')
		{
			if (Byte[0] == '?')
			{
				SigBytes.push_back({ true, 0 });
			}
			else
			{
				SigBytes.push_back({ false, (uint8_t)std::stoi(Byte, nullptr, 16) });
			}
			Byte = "";
		}
		else
		{
			Byte += Char;
		}
	}

	if (Byte[0] == '?')
	{
		SigBytes.push_back({ true, 0 });
	}
	else
	{
		SigBytes.push_back({ false, (uint8_t)std::stoi(Byte, nullptr, 16) });
	}

	uintptr_t Start = this->ExeBaseAddr;

	// https://wiki.alliedmods.net/Signature_Scanning
	MEMORY_BASIC_INFORMATION Mbi;

	if (!VirtualQuery((const void*)Start, &Mbi, sizeof(Mbi)))
		return 0;

	IMAGE_DOS_HEADER* DosHeader = (IMAGE_DOS_HEADER*)Mbi.AllocationBase;

#ifdef X64
#define NtHeadersStruct IMAGE_NT_HEADERS64
#else
#define NtHeadersStruct IMAGE_NT_HEADERS
#endif

	NtHeadersStruct* PeHeader = (NtHeadersStruct*)((uintptr_t)DosHeader + (uintptr_t)DosHeader->e_lfanew);

	uintptr_t ImageSize = (uintptr_t)PeHeader->OptionalHeader.SizeOfImage;
	uintptr_t End = Start + ImageSize;
	
	uintptr_t Current = Start;

	uint32_t FirstFourBytesOfSig =
		((uint32_t)SigBytes[0].Byte << 0 ) |
		((uint32_t)SigBytes[1].Byte << 8 ) |
		((uint32_t)SigBytes[2].Byte << 16) |
		((uint32_t)SigBytes[3].Byte << 24) ;

	bool MatchedFirstFourBytes = false;

	while (Current + SigBytes.size() <= End)
	{
		while (Current + SigBytes.size() <= End)
		{
			if (ReadAddr(uint32_t, Current) == FirstFourBytesOfSig)
			{
				MatchedFirstFourBytes = true;
				break;
			}
			Current++;
		}

		if (MatchedFirstFourBytes)
		{
			uintptr_t CurrentForSearch = Current + sizeof(uint32_t);
			while (CurrentForSearch - Current < SigBytes.size())
			{
				size_t Idx = CurrentForSearch - Current;
				if (ReadAddr(uint8_t, CurrentForSearch) != SigBytes[Idx])
				{
					Current++;
					MatchedFirstFourBytes = false;
					break;
				}
				CurrentForSearch++;
			}

		}
		
		if (MatchedFirstFourBytes)
		{
			return Current;
		}
	}

	return 0;
}

CMwNod* TwinkTrackmania::GetApp()
{
#ifndef TM1
	return ReadAddr(CMwNod*, this->ExeBaseAddr + O_APP);
#else
	return ReadAddr(CMwNod*, O_M_CMWNODMAIN_APP + O_APP);
#endif
}

CMwNod* TwinkTrackmania::GetViewport()
{
#ifndef TM1
	return ReadAddr(CMwNod*, (uintptr_t)GetApp() + O_M_CTRACKMANIA_VIEWPORT);
#else
	return ReadAddr(CMwNod*, O_M_CMWNODMAIN_VIEWPORT + O_APP);
#endif
}

uintptr_t TwinkTrackmania::GetDirectXDevice()
{
#ifndef TM1
	return ReadAddr(uintptr_t, this->ExeBaseAddr + O_D3DDEVICE);
#else
	return ReadAddr(uintptr_t, O_M_CVISIONVIEWPORTDX9_DEVICE + (uintptr_t)GetViewport());
#endif
}

CInputPort* TwinkTrackmania::GetInputPort()
{
	return ReadAddr(CInputPort*, (uintptr_t)GetApp() + O_M_CTRACKMANIA_INPUTPORT);
}

#ifdef GAMEBOX
bool TwinkTrackmania::GetIsIntroOver()
{
#ifndef TMU
	return ReadAddr(bool, this->ExeBaseAddr + O_ISINTROOVER);
#else
	return true;
#endif
}
#endif

#ifdef MANIAPLANET
uintptr_t TwinkTrackmania::GetDirectXSwapChain()
{
	return ReadAddr(uintptr_t, (uintptr_t)this->GetViewport() + O_M_CDX11VIEWPORT_D3DSWAPCHAIN);
}
#endif

#pragma optimize("", off)
void TwinkTrackmania::PushToStack(CMwStack* Stack, CMwMemberInfo* MemberInfo)
{
#ifdef GAMEBOX
	static CMwMemberInfo* OptimizationRemover = nullptr;
	if (!OptimizationRemover) OptimizationRemover = nullptr;

	if (Stack->m_Size != 0 and Stack->ppMemberInfos)
	{
		realloc(Stack->ppMemberInfos, (Stack->m_Size + 1) * sizeof(CMwMemberInfo*));

		Stack->ppMemberInfos[Stack->m_Size] = MemberInfo;

		Stack->m_Size++;
	}
	else
	{
		Stack->ppMemberInfos = new CMwMemberInfo* { MemberInfo };
		Stack->m_Size = 1;
	}
	OptimizationRemover = *Stack->ppMemberInfos;
#else
	if (Stack->m_Size < 2)
	{
		Stack->m_ContainedItems[Stack->m_Size] = CMwStack::Item{ MemberInfo, CMwStack::ITEM_MEMBER };
		Stack->m_Size++;
	}
	else
	{
		if (Stack->m_ExtraItemsCapacity != 0 and Stack->m_pExtraItems)
		{
			Stack->m_pExtraItems = (CMwStack::Item*)realloc(Stack->m_pExtraItems, (Stack->m_ExtraItemsCapacity + 1) * sizeof(CMwStack::Item));
		}
		else if (Stack->m_ExtraItemsCapacity == 0)
		{
			if (Stack->m_pExtraItems) free(Stack->m_pExtraItems);
			Stack->m_pExtraItems = (CMwStack::Item*)malloc((Stack->m_ExtraItemsCapacity + 1) * sizeof(CMwStack::Item));
		}
		if (Stack->m_pExtraItems) Stack->m_pExtraItems[Stack->m_ExtraItemsCapacity] = CMwStack::Item{ MemberInfo, CMwStack::ITEM_MEMBER };
		Stack->m_ExtraItemsCapacity++;
		Stack->m_Size++;
	}
#endif
}
#pragma optimize("", on)

// END Getters