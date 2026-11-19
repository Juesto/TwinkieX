#pragma once
#include "pch.h"
#include "ILoggert.h"
#include "TwinkLoggert/TwinkLoggert.h"
#include "AngelscriptLoggert/AngelscriptLoggert.h"
#include "CppLoggert/CppLoggert.h"

class TwinkLogs
{
	TwinkLogs operator=(TwinkLogs) = delete;
	TwinkLogs& operator=(TwinkLogs&) = delete;
	TwinkLogs&& operator=(TwinkLogs&&) = delete;

	friend class TwinkUi;
	friend class Twinkie;

protected:
	std::stringstream LogPreview;
	std::fstream LogFile;

	std::vector<ILoggert*> Loggerts;

private:
	void Clear();
	void PullFromLoggerts();
	void Render();
	void UpdateLogFileLoc(std::filesystem::path& Loc);

public:
	TwinkLogs();
	~TwinkLogs();

	void push_back(ILoggert* Loggert);
	void GetPreview(std::string& Result);
};