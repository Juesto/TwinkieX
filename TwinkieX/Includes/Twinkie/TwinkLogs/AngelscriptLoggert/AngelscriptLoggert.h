#pragma once
#include <Twinkie/TwinkLogs/ILoggert.h>

class AngelscriptLoggert : public ILoggert
{
	AngelscriptLoggert operator=(AngelscriptLoggert) = delete;
	AngelscriptLoggert& operator=(AngelscriptLoggert&) = delete;
	AngelscriptLoggert&& operator=(AngelscriptLoggert&&) = delete;
public:
	AngelscriptLoggert() : ILoggert(u8"\\$6f9", u8"ScriptEngine", u8"") {};
};

// Loggert only used by angelscript modules.
extern AngelscriptLoggert LoggertAngelscript;