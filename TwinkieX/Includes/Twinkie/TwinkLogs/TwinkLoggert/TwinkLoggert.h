#pragma once
#include <Twinkie/TwinkLogs/ILoggert.h>

class TwinkLoggert : public ILoggert
{
	TwinkLoggert operator=(TwinkLoggert) = delete;
	TwinkLoggert& operator=(TwinkLoggert&) = delete;
	TwinkLoggert&& operator=(TwinkLoggert&&) = delete;
public:
	TwinkLoggert() : ILoggert(u8"\\$f0f", u8"Twinkie", u8"") {};
};

// Loggert only for internal code. Do not use for C++ or Angelscript (try reading similar folders near me!)
extern TwinkLoggert LoggertTwink;