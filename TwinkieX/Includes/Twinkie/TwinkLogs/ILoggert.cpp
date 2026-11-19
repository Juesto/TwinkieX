#include "pch.h"
#include <Twinkie/TwinkLogs/ILoggert.h>

#define ILoggert_MakeFn(name, color, tag) void ILoggert::##name(std::u8string str) \
{ \
    std::string sStr((const char*)str.c_str()); \
	std::string sColorTag((const char*)ColorTag.c_str()); \
	std::string sIcon((const char*)Icon.c_str()); \
	std::string sTag((const char*)Tag.c_str()); \
	\
    std::string MessageFormattedPrev = std::format(color tag " {}[{}{}] \\$g{}\n", sColorTag, sIcon, sTag, sStr); \
	std::string MessageFormattedFile = std::format(tag " [{}{}] {}\n", sIcon, sTag, sStr); \
	\
	LoggertStreamForPreview << MessageFormattedPrev; \
	LoggertStreamForFile << MessageFormattedFile; \
}

#define ILoggert_MakeFnExtraColored(name, color, tag) void ILoggert::##name(std::u8string str) \
{ \
    std::string sStr((const char*)str.c_str()); \
	std::string sColorTag((const char*)ColorTag.c_str()); \
	std::string sIcon((const char*)Icon.c_str()); \
	std::string sTag((const char*)Tag.c_str()); \
	\
    std::string MessageFormattedPrev = std::format(color tag " {}[{}{}] " color "{}\n", sColorTag, sIcon, sTag, sStr); \
	std::string MessageFormattedFile = std::format(tag " [{}{}] {}\n", sIcon, sTag, sStr); \
	\
	LoggertStreamForPreview << MessageFormattedPrev; \
	LoggertStreamForFile << MessageFormattedFile; \
}

ILoggert_MakeFnExtraColored(trace, "\\$999", "[d]")
ILoggert_MakeFn(print, "\\$35e", "[i]")
ILoggert_MakeFnExtraColored(warn,  "\\$ff0", "[w]")
ILoggert_MakeFnExtraColored(error, "\\$f00", "[e]")