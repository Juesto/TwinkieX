#include "pch.h"
#include "HttpReq.h"
#pragma warning(disable : 4505 6262)

static std::u8string FromStr(const std::string& that)
{
    return std::u8string((const char8_t*)that.data(), that.size());
}

static std::string ToStr(const std::u8string& that)
{
    return std::string((const char*)that.data(), that.size());
}

static std::wstring FromStr(const std::u8string& that)
{
    return std::filesystem::path(that).wstring();
}

static std::u8string ToStr(const std::wstring& that)
{
    return std::filesystem::path(that).u8string();
}

template<typename T>
static std::basic_string<T>& StringoidToLower(std::basic_string<T>& Str)
{
    for (T& C : Str)
    {
        C = std::tolower(C);
    }

    return Str;
}

enum class HttpHeadersFrom : uint32_t
{
    Request = WINHTTP_QUERY_FLAG_REQUEST_HEADERS,
    Response = 0
};

static bool GetHeadersWinHttp(
    HINTERNET Request,
    std::unordered_map<std::u8string, std::u8string>& HeadersOut,
    std::u8string& Err,
    HttpHeadersFrom Where
)
{
    size_t ReqHeadersBufSz = 0;
    // No error check because when using WINHTTP_NO_OUTPUT_BUFFER, it always returns FALSE.
    WinHttpQueryHeaders(
        Request,
        WINHTTP_QUERY_RAW_HEADERS_CRLF | (uint32_t)Where,
        WINHTTP_HEADER_NAME_BY_INDEX,
        WINHTTP_NO_OUTPUT_BUFFER,
        (LPDWORD)&ReqHeadersBufSz,
        WINHTTP_NO_HEADER_INDEX
    );

    // Includes nasty things like verb and object.
    // CRLF-separated.
    std::vector<wchar_t> ReqHeadersBuf;
    ReqHeadersBuf.resize(ReqHeadersBufSz);
    if (!WinHttpQueryHeaders(
        Request,
        WINHTTP_QUERY_RAW_HEADERS_CRLF | (uint32_t)Where,
        WINHTTP_HEADER_NAME_BY_INDEX,
        ReqHeadersBuf.data(),
        (LPDWORD)&ReqHeadersBufSz,
        WINHTTP_NO_HEADER_INDEX
    ))
    {
        Err = u8"Could not query request headers.";
        return false;
    }

    std::vector<std::wstring> HeaderLines;
    {
        std::wstringstream HeaderWss(ReqHeadersBuf.data());
        std::wstring HeaderLine;
        size_t HeaderLineIdx = 0;
        while (std::getline(HeaderWss, HeaderLine, L'\r'))
        {
            // Remove extra \n and skip verb and object
            if (HeaderLineIdx > 0) HeaderLines.push_back(HeaderLine.substr(1, HeaderLine.size() - 1));
            HeaderLineIdx++;
        }
        // Remove empty lines from the back
        for (size_t RemovalHeaderLineIdx = HeaderLines.size() - 1; RemovalHeaderLineIdx != 0; RemovalHeaderLineIdx--)
        {
            if (HeaderLines[RemovalHeaderLineIdx].empty())
            {
                HeaderLines.erase(HeaderLines.begin() + RemovalHeaderLineIdx);
            }
            else break;
        }
    }

    for (auto& HeaderLine : HeaderLines)
    {
        std::vector<std::wstring> Tokens;
        size_t Pos = 0;
        std::wstring Token;
        while ((Pos = HeaderLine.find(L": ")) != std::wstring::npos) {
            Token = HeaderLine.substr(0, Pos);
            Tokens.push_back(Token);
            HeaderLine.erase(0, Pos + 2);
        }
        Tokens.push_back(HeaderLine);

        if (Tokens.size() < 2)
        {
            Err = u8"Invalid request header format.";
            return false;
        }
        else
        {
            auto Key = ToStr(Tokens[0]);
            StringoidToLower(Key);
            HeadersOut[Key] = ToStr(Tokens[1]);
        }
    }
    return true;
}

// Make an HTTP/HTTPS request and store the return data.
// This function is blocking. For a non-blocking method, use HttpRequest.
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
    const std::u8string& ContentType,
    const std::u8string& SaveToFilePath,
    bool Redirect
)
{
    {
        DownUpInfo.BytesToBeDownloaded = 0;
        DownUpInfo.BytesToBeUploaded = DataIn.size();
        DownUpInfo.DownloadedBytes = 0;
        DownUpInfo.UploadedBytes = 0;

        Err = u8"";
    }

    std::wstring WUrl = FromStr(Url);

    wchar_t ExtraInfo[8192]{ 0 };
    wchar_t Path[1024]{ 0 };
    wchar_t Host[512]{ 0 };
    wchar_t Password[512]{ 0 };
    wchar_t Username[128]{ 0 };

#define DefComp(c, v) Components.lpsz##c = v; Components.dw##c##Length = sizeof(v)
    URL_COMPONENTS Components{};
    Components.dwStructSize = sizeof(URL_COMPONENTS);
    DefComp(HostName, Host);
    DefComp(UrlPath, Path);
    DefComp(Password, Password);
    DefComp(UserName, Username);
    DefComp(ExtraInfo, ExtraInfo);

    if (!WinHttpCrackUrl(WUrl.c_str(), (DWORD)WUrl.size(), 0, &Components))
    {
        Err = u8"Could not parse URL.";
        return false;
    }

    auto Session = WinHttpOpen(
        FromStr(AgentInfo).c_str(),
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0
    );
    if (!Session || Session == INVALID_HANDLE_VALUE)
    {
        WinHttpCloseHandle(Session);
        Err = u8"WinHTTP failure while setting up session.";
        return false;
    }

    auto Connection = WinHttpConnect(Session, Host, Components.nPort, 0);
    if (!Connection || Connection == INVALID_HANDLE_VALUE)
    {
        WinHttpCloseHandle(Connection); WinHttpCloseHandle(Session);
        Err = u8"WinHTTP failure while setting up connection.";
        return false;
    }

    DWORD ReqFlags = (Components.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;

    wchar_t AcceptTypes[128];
    if (!ContentType.empty())
        memcpy_s(
            AcceptTypes,
            sizeof(AcceptTypes),
            FromStr(ContentType).c_str(),
            FromStr(ContentType).size() * sizeof(wchar_t)
        );
    wchar_t* pAcceptTypes = AcceptTypes;

    auto Request = WinHttpOpenRequest(
        Connection,
        FromStr(Verb).c_str(),
        Path[0] ? Path : L"/",
        nullptr,
        WINHTTP_NO_REFERER,
        ContentType.empty() ? WINHTTP_DEFAULT_ACCEPT_TYPES : (LPCWSTR*)&pAcceptTypes,
        ReqFlags
    );

    if (ReqHeadersInOut.size())
    {
        std::wstring CollectiveReqHeadersIn;
        for (auto& Header : ReqHeadersInOut)
        {
            CollectiveReqHeadersIn += FromStr(Header.first) + L": " + FromStr(Header.second) + L"\r\n";
        }
        // Remove last \r\n as docs say
        CollectiveReqHeadersIn = CollectiveReqHeadersIn.substr(0, CollectiveReqHeadersIn.size() - 2);

        WinHttpAddRequestHeaders(Request, CollectiveReqHeadersIn.c_str(), (DWORD)CollectiveReqHeadersIn.size(), WINHTTP_ADDREQ_FLAG_REPLACE);
    }

    unsigned long FeatureToDisable = WINHTTP_DISABLE_REDIRECTS;
    if (!Redirect) WinHttpSetOption(Request, WINHTTP_OPTION_DISABLE_FEATURE, &FeatureToDisable, sizeof(FeatureToDisable));

    if (!WinHttpSendRequest(
        Request,
        WINHTTP_NO_ADDITIONAL_HEADERS,
        0,
        WINHTTP_NO_REQUEST_DATA,
        0, (DWORD)DataIn.size(), 0))
    {
        WinHttpCloseHandle(Request); WinHttpCloseHandle(Connection); WinHttpCloseHandle(Session);
        Err = u8"Sending HTTP request failed.";
        return false;
    }

    size_t DataWrittenToRequest = 0;
    if (!DataIn.empty())
    {
        WinHttpWriteData(Request, DataIn.data(), (DWORD)(DataIn.size() * sizeof(uint8_t)), (LPDWORD)&DataWrittenToRequest);
        if (DataWrittenToRequest != DataIn.size() * sizeof(uint8_t))
        {
            WinHttpCloseHandle(Request); WinHttpCloseHandle(Connection); WinHttpCloseHandle(Session);
            Err = u8"Not all data was written into request.";
            return false;
        }

        // TODO: Is this accurate?
        DownUpInfo.UploadedBytes = DataWrittenToRequest;
    }

    if (!WinHttpReceiveResponse(Request, nullptr))
    {
        WinHttpCloseHandle(Request); WinHttpCloseHandle(Connection); WinHttpCloseHandle(Session);
        Err = u8"Recieving response failed.";
        return false;
    }

    GetHeadersWinHttp(Request, ReqHeadersInOut, Err, HttpHeadersFrom::Request);
    GetHeadersWinHttp(Request, ResponseHeadersOut, Err, HttpHeadersFrom::Response);

    if (ResponseHeadersOut.contains(u8"content-length"))
    {
        DownUpInfo.BytesToBeDownloaded = std::stoull(FromStr(ResponseHeadersOut[u8"content-length"]));
    }

    size_t DataAvailable = 0;
    size_t DataRead = 0;
    bool CouldWriteToFile = true;
    std::ofstream File;
    if (!SaveToFilePath.empty()) File.open(std::filesystem::path(SaveToFilePath), std::ios::binary | std::ios::out);
    CouldWriteToFile = File.is_open();
    if (!CouldWriteToFile) Err = u8"Could not write to file.";
    while (WinHttpQueryDataAvailable(Request, (LPDWORD)&DataAvailable) && DataAvailable)
    {
        size_t AlreadyWritten = DataOut.size();
        DataOut.resize(AlreadyWritten + DataAvailable);

        WinHttpReadData(
            Request,
            DataOut.data() + AlreadyWritten,
            (DWORD)DataAvailable,
            (LPDWORD)&DataRead
        );

        if (!SaveToFilePath.empty() && CouldWriteToFile)
        {
            try
            {
                File.write((const char*)(DataOut.data() + AlreadyWritten), DataOut.size());
            }
            catch (...)
            {
                Err = u8"Could not write to file.";
                CouldWriteToFile = false;
            }
        }

        DownUpInfo.DownloadedBytes += DataRead;

        DataOut.resize(AlreadyWritten + DataRead);
    }

    WinHttpCloseHandle(Request); WinHttpCloseHandle(Connection); WinHttpCloseHandle(Session);

    return CouldWriteToFile;
#undef DefComp
}

DWORD WINAPI HttpThread(LPVOID lpParameter)
{
    HttpRequest* Req = (HttpRequest*)lpParameter;
    if (!Req || Req->Url.empty()) return FALSE;

    bool Result = MakeRawHttpRequest(
        Req->Url,
        Req->AgentInfo,
        Req->Verb,
        Req->DataIn,
        Req->DataOut,
        Req->ReqHeadersInOut,
        Req->ResponseHeadersOut,
        Req->LastErr,
        Req->DownUpInfo,
        Req->ContentType,
        Req->SaveToFilePath,
        Req->Redirect
    );

    Req->Done = true;
    Req->RunningThread = NULL;

    return Result;
}

bool HttpRequest::Start(const std::u8string& pUrl)
{
    this->Done = false;
    if (!this->DataOut.empty()) this->DataOut.resize(0);
    if (!pUrl.empty()) this->Url = pUrl;
    if (!(this->RunningThread = CreateThread(NULL, 0, HttpThread, this, 0, NULL)))
        return false;
    return true;
}

bool HttpRequest::StartToFile(const std::u8string& path)
{
    this->SaveToFilePath = path;
    return Start();
}

bool HttpRequest::Finished()
{
    return Done;
}

size_t HttpRequest::ProgressDownload()
{
    return DownUpInfo.DownloadedBytes;
}

size_t HttpRequest::ProgressDownloadTotal()
{
    return DownUpInfo.BytesToBeDownloaded;
}

size_t HttpRequest::ProgressUpload()
{
    return DownUpInfo.UploadedBytes;
}

size_t HttpRequest::ProgressUploadTotal()
{
    return DownUpInfo.BytesToBeUploaded;
}

void HttpRequest::Cancel()
{
    // TODO: probably shouldn't use TerminateThread here, but HttpThread is safe from leaks
    TerminateThread(this->RunningThread, 0);
    this->Done = true;
}

void HttpRequest::AddRef()
{
    this->RefCount++;
}

void HttpRequest::RemoveRef()
{
    if (!--this->RefCount)
        delete this;
}

HttpRequest::~HttpRequest()
{
    if (!this->Done)
    {
        this->Cancel();
    }
}