#include "pch.h"
#include "Twinkie/TwinkLogs/TwinkLogs.h"

TwinkLogs::TwinkLogs() 
{
	push_back(&LoggertTwink);
	push_back(&LoggertCpp);
	push_back(&LoggertAngelscript);
}

TwinkLogs::~TwinkLogs()
{
	LogFile.flush();
	LogFile.close();
}

void TwinkLogs::push_back(ILoggert* Loggert)
{
	Loggerts.push_back(Loggert);
}

void TwinkLogs::GetPreview(std::string& Result)
{
	LogPreview.seekp(0);
	Result = LogPreview.str();
}

void TwinkLogs::Clear()
{
	LogPreview.clear();
}

void TwinkLogs::PullFromLoggerts()
{
	for (auto& Loggert : Loggerts)
	{
		Loggert->LoggertStreamForPreview.seekg(0);
		Loggert->LoggertStreamForFile.seekg(0);

		LogPreview << Loggert->LoggertStreamForPreview.str();
		LogFile << Loggert->LoggertStreamForFile.str();

		Loggert->LoggertStreamForPreview.str("");
		Loggert->LoggertStreamForPreview.clear();
		
		Loggert->LoggertStreamForFile.str("");
		Loggert->LoggertStreamForFile.clear();
	}
}

void TwinkLogs::Render()
{
	using namespace ImGui;

	PullFromLoggerts();

	Begin("Logs##Twinkie", nullptr, ImGuiWindowFlags_MenuBar);

	BeginMenuBar();

	if (MenuItem("Clear"))
	{
		LogPreview.str("");
		LogPreview.clear();
	}

	EndMenuBar();

	auto LastPos = LogPreview.tellg();
	LogPreview.seekg(0);

	std::string Line;
	while (std::getline(LogPreview, Line))
	{
		Text(Line.c_str());
	}

	// clear doesn't clear the stream and i think someone deserves a real good firing from the c++ standard committee
	// clear removes any flags like "is eof" and "has read failed"
	LogPreview.clear();
	LogPreview.seekg(LastPos);

	End();
}

void TwinkLogs::UpdateLogFileLoc(std::filesystem::path& DocumentsFolderLoc)
{
	auto LogFileLoc = DocumentsFolderLoc / "TwinkieX\\Twinkie" TWINKIEX_GAME ".log";

	if (std::filesystem::exists(LogFileLoc))
	{
		std::filesystem::rename(LogFileLoc, DocumentsFolderLoc / "TwinkieX\\Twinkie" TWINKIEX_GAME "-Old.log");
	}
	else
	{
		std::ofstream(LogFileLoc).close();
	}

	LogFile.open(LogFileLoc, std::ios::out);
}