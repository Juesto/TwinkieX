
#pragma once
#include "pch.h"
#include "Version.h"

DWORD WINAPI HttpThread(LPVOID lpParameter);

namespace Gecko::Exports
{
    struct GeckoAutoExport;
    namespace Net
    {
        void Registrar(asIScriptEngine*);
    }
}

struct HttpDownUpInfo
{
    size_t DownloadedBytes = 0;
    size_t BytesToBeDownloaded = 0;
    size_t UploadedBytes = 0;
    size_t BytesToBeUploaded = 0;
};

enum class HttpMethod : uint32_t
{
    Get, 
    Post, 
    Head, 
    Put,
    Delete,
    Patch
};

bool MakeRawHttpRequest(
    const std::u8string& Url,
    const std::u8string& AgentInfo,
    const std::u8string& Verb,
    const std::vector<uint8_t>& DataIn,
    std::vector<uint8_t>& DataOut,
    std::unordered_map<std::u8string, std::u8string>& ReqHeadersInOut,
    std::unordered_map<std::u8string, std::u8string>& ResponseHeadersOut,
    std::u8string& Err,
    HttpDownUpInfo& DownUpInfo,
    const std::u8string& ContentType = u8"",
    const std::u8string& SaveToFilePath = u8"",
    bool Redirect = true
);

// A non-blocking HTTP/HTTPS request. For a blocking method, use MakeRawHttpRequest.
class HttpRequest
{
    bool Done = true;
    HttpDownUpInfo DownUpInfo;
    std::u8string SaveToFilePath;
    HANDLE RunningThread = NULL;
    size_t RefCount = 0;

    void AddRef();
    void RemoveRef();

    friend struct Gecko::Exports::GeckoAutoExport;
    friend void Gecko::Exports::Net::Registrar(asIScriptEngine*);
    friend DWORD WINAPI HttpThread(LPVOID lpParameter);
public:
    std::u8string Url;
    // Don't change unless you know what you're doing!
    std::u8string AgentInfo = u8"TwinkieX/" TWINKIEX_VERSION_HTTP " / jailmanguy1[at]gmail.com";
    std::u8string Verb = u8"GET";
    std::vector<uint8_t> DataIn;
    std::vector<uint8_t> DataOut;
    std::unordered_map<std::u8string, std::u8string> ReqHeadersInOut;
    std::unordered_map<std::u8string, std::u8string> ResponseHeadersOut;
    std::u8string LastErr;
    std::u8string ContentType;
    bool Redirect = true;

    HttpRequest(std::u8string Url) : Url(Url) {}
    HttpRequest() {}

    ~HttpRequest();

    bool Start(const std::u8string& pUrl = u8"");
    bool StartToFile(const std::u8string& path);
    bool Finished();

    size_t ProgressDownload();
    size_t ProgressDownloadTotal();
    size_t ProgressUpload();
    size_t ProgressUploadTotal();

    void Cancel();
};