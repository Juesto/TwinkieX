#pragma once
#include "pch.h"

// Represents a "logging profile" for an internal system.
// TODO: Add timestamp
// NOTE: Icon (if added) needs an extra space at the end to look good.
// [{Level}] {Color}[{Icon}{Tag}]$g {Message}
class ILoggert
{
	friend class TwinkLogs;

	std::stringstream LoggertStreamForPreview{};
	std::stringstream LoggertStreamForFile{};
public:
	ILoggert operator=(ILoggert) = delete;
	ILoggert& operator=(ILoggert&) = delete;
	ILoggert&& operator=(ILoggert&&) = delete;

	// Represented as TrackmaniaFormatting
	std::u8string ColorTag;

	std::u8string Tag;
	std::u8string Icon;

	bool ResetColorForMessage = false;

	ILoggert();
	ILoggert(std::u8string ColorTag, std::u8string Tag, std::u8string Icon, bool ResetColorForMessage = false) : 
		ColorTag(ColorTag), 
		Tag(Tag), 
		Icon(Icon), 
		ResetColorForMessage(ResetColorForMessage) {};

	virtual ~ILoggert() {};

	virtual void trace(std::u8string);
	virtual void print(std::u8string);
	virtual void warn(std::u8string);
	virtual void error(std::u8string);
};