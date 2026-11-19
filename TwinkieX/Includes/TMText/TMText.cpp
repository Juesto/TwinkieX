#include "pch.h"
#include "TMText.h"

namespace TrackmaniaText
{
	std::string ValidTrackmaniaTextFormatSpecifierChars = "<>ihownmtslgzpabcdef0123456789";

	constexpr char CharToLower(char Char)
	{
		return Char >= 'A' and Char <= 'Z' ? Char | 32 : Char;
	}

	FormatSpecifier::FormatSpecifier(std::string FormatSpec, size_t Idx)
	{
		char TypeOfSpecifierDecider = CharToLower(FormatSpec[1]);
		if (TrackmaniaText_IsHexadecimal(TypeOfSpecifierDecider))
		{
			this->Type = FormatSpecifierType_Color;
		}
		else if (ValidTrackmaniaTextFormatSpecifierChars.find(TypeOfSpecifierDecider) == std::string::npos)
		{
			this->Type = FormatSpecifierType_NotSupported;
		}
		else
		{
			this->Type = (FormatSpecifierType)TypeOfSpecifierDecider;
		}

		this->Code = FormatSpec;
		this->Idx = Idx;
#ifdef _M_X64
		this->Pad = 0;
#endif
	}

	void FormatSpecifierSearchResult::push_back(std::string FormatSpec, size_t Idx)
	{
		Specifiers.push_back(FormatSpecifier( FormatSpec, Idx ));
	}

	std::vector<FormatSpecifier>::iterator FormatSpecifierSearchResult::begin()
	{
		return Specifiers.begin();
	}

	std::vector<FormatSpecifier>::iterator FormatSpecifierSearchResult::end()
	{
		return Specifiers.end();
	}

    // Format specifiers start with "\$" (e.g. "\$f00", "\$o", "\$z"). A lone '$' or '\' is plain text.
    // "\$$" produces a literal '$'. A trailing "\$" with nothing after it is dropped.
    // Specifier codes are stored as "$<code>" (without the backslash), so FormatSpecifier and
    // FormatState::ApplyFormatSpecifier don't need to know about the prefix.
    // NOTE: ImFontCalcTextSizeEx (imgui_draw.cpp) mirrors this logic to map clean offsets back to raw text.
    FormatSpecifierSearchResult FindAllFormatSpecifiers(std::string Str)
    {
        FormatSpecifierSearchResult Founds = {};
        const size_t Len = Str.length();

        size_t CharIdx = 0;
        while (CharIdx < Len)
        {
            if (Str[CharIdx] != '\\' || CharIdx + 1 >= Len || Str[CharIdx + 1] != '$')
            {
                Founds.OriginalText.push_back(Str[CharIdx]);
                CharIdx++;
                continue;
            }

            CharIdx += 2; // Skip "\$"
            if (CharIdx >= Len) break;

            char Code = Str[CharIdx];
            if (Code == '$')
            {
                Founds.OriginalText.push_back('$');
                CharIdx++;
                continue;
            }

            // Colors are 3 chars ("\$f00"), everything else is 1 char ("\$o").
            size_t CodeLen = (TrackmaniaText_IsHexadecimal(Code)) ? 3 : 1;
            Founds.push_back("$" + Str.substr(CharIdx, CodeLen), Founds.OriginalText.length());
            CharIdx += CodeLen;
        }

        return Founds;
    }
}