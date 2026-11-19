#pragma once
#include <Twinkie/TwinkLogs/ILoggert.h>

class CppLoggert : public ILoggert
{
	CppLoggert operator=(CppLoggert) = delete;
	CppLoggert& operator=(CppLoggert&) = delete;
	CppLoggert&& operator=(CppLoggert&&) = delete;
public:
	CppLoggert() : ILoggert(u8"\\$35e", u8"C++", u8"") {};
};

// Loggert for C++ modules. Not for use within internal non-module code.
extern CppLoggert LoggertCpp;