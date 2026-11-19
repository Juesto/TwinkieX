#include "pch.h"
#include "Net.h"
#include <Gecko/Exports/GeckoAutoExport.h>
#include <Gecko/Exports/Global/MemoryBuffer/MemoryBuffer.h>
#include <fstream>
#include <filesystem>

namespace Gecko::Exports::Net
{
	static std::vector<GeckoExport> s_BaseExports;
	static std::vector<GeckoExport> s_Exports;

#define NET_HTTPREQUEST_PROP(Decl_, Offset_)        GECKO_EXPORT_PROP(s_Exports, "Net", "HttpRequest", Decl_, Offset_)
#define NET_HTTPREQUEST_METHOD(Decl_, Ptr_, Conv_)          GECKO_EXPORT_METHOD(s_BaseExports, "Net", "HttpRequest", Decl_, Ptr_, Conv_)
#define NET_HTTPREQUEST_BEHAVIOUR(Beh_, Decl_, Ptr_, Conv_) GECKO_EXPORT_BEHAVIOUR(s_BaseExports, "Net", "HttpRequest", Beh_, Decl_, Ptr_, Conv_)
#define NET_FN(Decl_, Fn_) GECKO_EXPORT_FN(s_Exports, "Net", Decl_, Fn_)
	// A comma. For use with NET_HTTPMETHOD_ENUM.
#define C ,
	// Usage: NET_HTTPMETHOD_ENUM({ "Value1" C "Value2" }, { 0, 1 })
	// Don't ask.
#define NET_HTTPMETHOD_ENUM(Names_, ...) GECKO_EXPORT_ENUM(s_BaseExports, Net, HttpMethod, Names_, __VA_ARGS__)

	static const char8_t* s_HttpMethodVerbs[] = { u8"GET", u8"POST", u8"HEAD", u8"PUT", u8"DELETE", u8"PATCH" };

	static std::u8string FromStr(const std::string& that)
	{
		return std::u8string((const char8_t*)that.data(), that.size());
	}

	static std::string ToStr(const std::u8string& that)
	{
		return std::string((const char*)that.data(), that.size());
	}

	static std::string FromBytes(const std::vector<uint8_t>& Bytes)
	{
		return std::string((const char*)Bytes.data(), Bytes.size());
	}

	static void PutStrIntoBytes(std::vector<uint8_t>& Bytes, const std::string& Str)
	{
		Bytes.assign((const uint8_t*)Str.data(), (const uint8_t*)Str.data() + Str.size());
	}

	static HttpRequest* StartNewRequest(const std::string& Url, const char8_t* Verb, const std::string& Data, const std::string& ContentType)
	{
		HttpRequest* Req = new HttpRequest(FromStr(Url));
		Req->Verb = Verb;
		PutStrIntoBytes(Req->DataIn, Data);
		if (!ContentType.empty()) Req->ReqHeadersInOut[u8"content-type"] = FromStr(ContentType);
		Req->Start();
		return Req;
	}

	/*
	!CompatMeta
	for     Net
	with    HttpMethod
	as      HttpMethod
	being   enum
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	NET_HTTPMETHOD_ENUM({"Get" C "Post" C "Head" C "Put" C "Delete" C "Patch"}, {0, 1, 2, 3, 4, 5});

	/*
	!CompatMeta
	for     Net
	with    string Url
	as      string Url
	of      HttpRequest
	being   prop
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	NET_HTTPREQUEST_PROP("string Url", asOFFSET(HttpRequest, Url));

	/*
	!CompatMeta
	for     Net
	with    bool Redirect
	as      bool Redirect
	being   prop
	of      HttpRequest
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	NET_HTTPREQUEST_PROP("bool Redirect", asOFFSET(HttpRequest, Redirect));

	/*
	!CompatMeta
	for     Net
	with    Net::HttpMethod Method
	as      HttpMethod Method
	being   prop
	of      HttpRequest
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	static int HttpRequest_get_Method(HttpRequest* Req)
	{
		for (int MethodIdx = 0; MethodIdx < (int)std::size(s_HttpMethodVerbs); MethodIdx++)
		{
			if (Req->Verb == s_HttpMethodVerbs[MethodIdx]) return MethodIdx;
		}
		return 0;
	}

	static void HttpRequest_set_Method(int Method, HttpRequest* Req)
	{
		if (Method < 0 || Method >= (int)std::size(s_HttpMethodVerbs)) return;
		Req->Verb = s_HttpMethodVerbs[Method];
	}
	NET_HTTPREQUEST_METHOD("HttpMethod get_Method() property", asFUNCTION(HttpRequest_get_Method), asCALL_CDECL_OBJLAST);
	NET_HTTPREQUEST_METHOD("void set_Method(HttpMethod) property", asFUNCTION(HttpRequest_set_Method), asCALL_CDECL_OBJLAST);

	/*
	!CompatMeta
	for     Net
	with    string Body
	as      string Body
	being   prop
	of      HttpRequest
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	static std::string HttpRequest_get_Body(HttpRequest* Req)
	{
		return FromBytes(Req->DataIn);
	}

	static void HttpRequest_set_Body(const std::string& Body, HttpRequest* Req)
	{
		PutStrIntoBytes(Req->DataIn, Body);
	}
	NET_HTTPREQUEST_METHOD("string get_Body() property", asFUNCTION(HttpRequest_get_Body), asCALL_CDECL_OBJLAST);
	NET_HTTPREQUEST_METHOD("void set_Body(const string&in) property", asFUNCTION(HttpRequest_set_Body), asCALL_CDECL_OBJLAST);

	/*
	!CompatMeta
	for     Net
	with    Net::HttpRequest@ HttpRequest()
	as      Net::HttpRequest@ HttpRequest()
	being   behavior
	of      HttpRequest
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	static HttpRequest* HttpRequest_Factory()
	{
		return new HttpRequest();
	}
	NET_HTTPREQUEST_BEHAVIOUR(asBEHAVE_FACTORY, "HttpRequest@+ f()", asFUNCTION(HttpRequest_Factory), asCALL_CDECL);

	/*
	!CompatMeta
	for     Net
	with    Net::HttpRequest@ HttpRequest(const string&in url)
	as      Net::HttpRequest@ HttpRequest(const string&in url)
	being   behavior
	of      HttpRequest
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	static HttpRequest* HttpRequest_FactoryUrl(const std::string& Url)
	{
		return new HttpRequest(FromStr(Url));
	}
	NET_HTTPREQUEST_BEHAVIOUR(asBEHAVE_FACTORY, "HttpRequest@+ f(const string&in url)", asFUNCTION(HttpRequest_FactoryUrl), asCALL_CDECL);

	/*
	!CompatMeta
	for     Net
	with    Net::HttpRequest@ HttpRequest(const string&in url, const string&in body)
	as      Net::HttpRequest@ HttpRequest(const string&in url, const string&in body)
	being   behavior
	of      HttpRequest
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	static HttpRequest* HttpRequest_FactoryUrlBody(const std::string& Url, const std::string& Body)
	{
		HttpRequest* Req = new HttpRequest(FromStr(Url));
		Req->Verb = u8"POST";
		PutStrIntoBytes(Req->DataIn, Body);
		return Req;
	}
	NET_HTTPREQUEST_BEHAVIOUR(asBEHAVE_FACTORY, "HttpRequest@+ f(const string&in url, const string&in body)", asFUNCTION(HttpRequest_FactoryUrlBody), asCALL_CDECL);

	/*
	!CompatMeta
	for     Net
	with    awaitable@ Start()
	as      bool Start()
	being   method
	of      HttpRequest
	declare partial
	since   v0.1.1
	reason  awaitable is not implemented. Returns whether the request could be started.
	!CompatMeta
	*/
	static bool HttpRequest_Start(HttpRequest* Req)
	{
		return Req->Start();
	}
	NET_HTTPREQUEST_METHOD("bool Start()", asFUNCTION(HttpRequest_Start), asCALL_CDECL_OBJLAST);

	/*
	!CompatMeta
	for     Net
	with    awaitable@ StartToFile(const string&in path)
	as      bool StartToFile(const string&in path)
	being   method
	of      HttpRequest
	declare partial
	since   v0.1.1
	reason  awaitable is not implemented. Returns whether the request could be started.
	!CompatMeta
	*/
	static bool HttpRequest_StartToFile(const std::string& Path, HttpRequest* Req)
	{
		return Req->StartToFile(FromStr(Path));
	}
	NET_HTTPREQUEST_METHOD("bool StartToFile(const string&in path)", asFUNCTION(HttpRequest_StartToFile), asCALL_CDECL_OBJLAST);

	/*
	!CompatMeta
	for     Net
	with    bool Finished()
	as      bool Finished()
	being   method
	of      HttpRequest
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	NET_HTTPREQUEST_METHOD("bool Finished()", asMETHOD(HttpRequest, Finished), asCALL_THISCALL);

	/*
	!CompatMeta
	for     Net
	with    int64 ProgressDownload()
	as      int64 ProgressDownload()
	being   method
	of      HttpRequest
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	static int64_t HttpRequest_ProgressDownload(HttpRequest* Req)
	{
		return (int64_t)Req->ProgressDownload();
	}
	NET_HTTPREQUEST_METHOD("int64 ProgressDownload()", asFUNCTION(HttpRequest_ProgressDownload), asCALL_CDECL_OBJLAST);

	/*
	!CompatMeta
	for     Net
	with    int64 ProgressDownloadTotal()
	as      int64 ProgressDownloadTotal()
	being   method
	of      HttpRequest
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	static int64_t HttpRequest_ProgressDownloadTotal(HttpRequest* Req)
	{
		return (int64_t)Req->ProgressDownloadTotal();
	}
	NET_HTTPREQUEST_METHOD("int64 ProgressDownloadTotal()", asFUNCTION(HttpRequest_ProgressDownloadTotal), asCALL_CDECL_OBJLAST);

	/*
	!CompatMeta
	for     Net
	with    int64 ProgressUpload()
	as      int64 ProgressUpload()
	being   method
	of      HttpRequest
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	static int64_t HttpRequest_ProgressUpload(HttpRequest* Req)
	{
		return (int64_t)Req->ProgressUpload();
	}
	NET_HTTPREQUEST_METHOD("int64 ProgressUpload()", asFUNCTION(HttpRequest_ProgressUpload), asCALL_CDECL_OBJLAST);

	/*
	!CompatMeta
	for     Net
	with    int64 ProgressUploadTotal()
	as      int64 ProgressUploadTotal()
	being   method
	of      HttpRequest
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	static int64_t HttpRequest_ProgressUploadTotal(HttpRequest* Req)
	{
		return (int64_t)Req->ProgressUploadTotal();
	}
	NET_HTTPREQUEST_METHOD("int64 ProgressUploadTotal()", asFUNCTION(HttpRequest_ProgressUploadTotal), asCALL_CDECL_OBJLAST);

	/*
	!CompatMeta
	for     Net
	with    void Cancel()
	as      void Cancel()
	being   method
	of      HttpRequest
	declare partial
	since   v0.1.1
	reason  Does not throw an exception when the request has not been started.
	!CompatMeta
	*/
	NET_HTTPREQUEST_METHOD("void Cancel()", asMETHOD(HttpRequest, Cancel), asCALL_THISCALL);

	/*
	!CompatMeta
	for     Net
	with    string ResponseHeader(const string&in key)
	as      string ResponseHeader(const string&in key)
	being   method
	of      HttpRequest
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	static std::string HttpRequest_ResponseHeader(const std::string& Key, HttpRequest* Req)
	{
		if (!Req->Finished()) return "";

		std::u8string LowerKey = FromStr(Key);
		for (char8_t& Char : LowerKey)
		{
			if (Char >= u8'A' && Char <= u8'Z') Char = (char8_t)(Char | 32);
		}

		auto Header = Req->ResponseHeadersOut.find(LowerKey);
		if (Header == Req->ResponseHeadersOut.end()) return "";
		return ToStr(Header->second);
	}
	NET_HTTPREQUEST_METHOD("string ResponseHeader(const string&in key)", asFUNCTION(HttpRequest_ResponseHeader), asCALL_CDECL_OBJLAST);

	/*
	!CompatMeta
	for     Net
	with    string String()
	as      string String()
	being   method
	of      HttpRequest
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	static std::string HttpRequest_String(HttpRequest* Req)
	{
		if (!Req->Finished()) return "";
		return FromBytes(Req->DataOut);
	}
	NET_HTTPREQUEST_METHOD("string String()", asFUNCTION(HttpRequest_String), asCALL_CDECL_OBJLAST);

	/*
	!CompatMeta
	for     Net
	with    void SaveToFile(const string&in path)
	as      void SaveToFile(const string&in path)
	being   method
	of      HttpRequest
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	static void HttpRequest_SaveToFile(const std::string& Path, HttpRequest* Req)
	{
		if (!Req->Finished())
		{
			// TODO: Raise an exception here and elsewhere
			return;
		}

		std::ofstream File(std::filesystem::path(FromStr(Path)), std::ios::binary | std::ios::out);
		if (!File.is_open())
		{
			return;
		}

		File.write((const char*)Req->DataOut.data(), (std::streamsize)Req->DataOut.size());
		if (!File.good()) return;
	}
	NET_HTTPREQUEST_METHOD("void SaveToFile(const string&in path)", asFUNCTION(HttpRequest_SaveToFile), asCALL_CDECL_OBJLAST);

	/*
	!CompatMeta
	for     Net
	with    MemoryBuffer@ Buffer()
	as      MemoryBuffer@ Buffer()
	being   method
	of      HttpRequest
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	static MemoryBuffer* HttpRequest_Buffer(HttpRequest* Req)
	{
		return new MemoryBuffer(Req->DataOut.data(), Req->DataOut.size());
	}
	NET_HTTPREQUEST_METHOD("MemoryBuffer@+ Buffer()", asFUNCTION(HttpRequest_Buffer), asCALL_CDECL_OBJLAST);

	/*
	!CompatMeta
	for     Net
	with    HttpRequest@ HttpGet(const string&in url)
	as      HttpRequest@ HttpGet(const string&in url)
	being   function
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	static HttpRequest* HttpGet(const std::string& Url)
	{
		return StartNewRequest(Url, u8"GET", "", "");
	}
	NET_FN("HttpRequest@+ HttpGet(const string&in url)", HttpGet);

	/*
	!CompatMeta
	for     Net
	with    HttpRequest@ HttpPost(const string&in url, const string&in data = "", const string&in contentType = "application/x-www-form-urlencoded")
	as      HttpRequest@ HttpPost(const string&in url, const string&in data = "", const string&in contentType = "application/x-www-form-urlencoded")
	being   function
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	static HttpRequest* HttpPost(const std::string& Url, const std::string& Data, const std::string& ContentType)
	{
		return StartNewRequest(Url, u8"POST", Data, ContentType);
	}
	NET_FN("HttpRequest@+ HttpPost(const string&in url, const string&in data = \"\", const string&in contentType = \"application/x-www-form-urlencoded\")", HttpPost);

	/*
	!CompatMeta
	for     Net
	with    HttpRequest@ HttpHead(const string&in url)
	as      HttpRequest@ HttpHead(const string&in url)
	being   function
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	static HttpRequest* HttpHead(const std::string& Url)
	{
		return StartNewRequest(Url, u8"HEAD", "", "");
	}
	NET_FN("HttpRequest@+ HttpHead(const string&in url)", HttpHead);

	/*
	!CompatMeta
	for     Net
	with    HttpRequest@ HttpPut(const string&in url, const string&in data = "", const string&in contentType = "application/x-www-form-urlencoded")
	as      HttpRequest@ HttpPut(const string&in url, const string&in data = "", const string&in contentType = "application/x-www-form-urlencoded")
	being   function
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	static HttpRequest* HttpPut(const std::string& Url, const std::string& Data, const std::string& ContentType)
	{
		return StartNewRequest(Url, u8"PUT", Data, ContentType);
	}
	NET_FN("HttpRequest@+ HttpPut(const string&in url, const string&in data = \"\", const string&in contentType = \"application/x-www-form-urlencoded\")", HttpPut);

	/*
	!CompatMeta
	for     Net
	with    HttpRequest@ HttpDelete(const string&in url)
	as      HttpRequest@ HttpDelete(const string&in url)
	being   function
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	static HttpRequest* HttpDelete(const std::string& Url)
	{
		return StartNewRequest(Url, u8"DELETE", "", "");
	}
	NET_FN("HttpRequest@+ HttpDelete(const string&in url)", HttpDelete);

	/*
	!CompatMeta
	for     Net
	with    HttpRequest@ HttpPatch(const string&in url, const string&in data = "", const string&in contentType = "application/x-www-form-urlencoded")
	as      HttpRequest@ HttpPatch(const string&in url, const string&in data = "", const string&in contentType = "application/x-www-form-urlencoded")
	being   function
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	static HttpRequest* HttpPatch(const std::string& Url, const std::string& Data, const std::string& ContentType)
	{
		return StartNewRequest(Url, u8"PATCH", Data, ContentType);
	}
	NET_FN("HttpRequest@+ HttpPatch(const string&in url, const string&in data = \"\", const string&in contentType = \"application/x-www-form-urlencoded\")", HttpPatch);

	/*
	!CompatMeta
	for     Net
	with    string UrlEncode(const string&in str)
	as      string UrlEncode(const string&in str)
	being   function
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	static std::string UrlEncode(const std::string& Str)
	{
		static const char* HexDigits = "0123456789ABCDEF";

		std::string Encoded;
		Encoded.reserve(Str.size() * 3);
		for (char Char : Str)
		{
			uint8_t Byte = (uint8_t)Char;
			bool Unreserved =
				(Byte >= 'A' && Byte <= 'Z') ||
				(Byte >= 'a' && Byte <= 'z') ||
				(Byte >= '0' && Byte <= '9') ||
				Byte == '-' || Byte == '_' || Byte == '.' || Byte == '~';

			if (Unreserved)
			{
				Encoded.push_back(Char);
			}
			else
			{
				Encoded.push_back('%');
				Encoded.push_back(HexDigits[Byte >> 4]);
				Encoded.push_back(HexDigits[Byte & 0xF]);
			}
		}
		return Encoded;
	}
	NET_FN("string UrlEncode(const string&in str)", UrlEncode);

	/*
	!CompatMeta
	for     Net
	with    string UrlDecode(const string&in str)
	as      string UrlDecode(const string&in str)
	being   function
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	static int HexDigitValue(char Char)
	{
		if (Char >= '0' && Char <= '9') return Char - '0';
		if (Char >= 'a' && Char <= 'f') return Char - 'a' + 10;
		if (Char >= 'A' && Char <= 'F') return Char - 'A' + 10;
		return -1;
	}

	static std::string UrlDecode(const std::string& Str)
	{
		std::string Decoded;
		Decoded.reserve(Str.size());
		for (size_t CharIdx = 0; CharIdx < Str.size(); CharIdx++)
		{
			if (Str[CharIdx] == '%' && CharIdx + 2 < Str.size())
			{
				int High = HexDigitValue(Str[CharIdx + 1]);
				int Low = HexDigitValue(Str[CharIdx + 2]);
				if (High != -1 && Low != -1)
				{
					Decoded.push_back((char)((High << 4) | Low));
					CharIdx += 2;
					continue;
				}
			}
			Decoded.push_back(Str[CharIdx]);
		}
		return Decoded;
	}
	NET_FN("string UrlDecode(const string&in str)", UrlDecode);

	void Registrar(asIScriptEngine* Engine)
	{
		Engine->SetDefaultNamespace("Net");
		Engine->RegisterObjectType("HttpRequest", sizeof(HttpRequest), asOBJ_REF);

		/*
		!CompatMeta
		for     Net
		with    void $beh5()
		as      void $beh5()
		being   behavior
		of      HttpRequest
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		Engine->RegisterObjectBehaviour("HttpRequest", asBEHAVE_ADDREF, "void f()", asMETHOD(HttpRequest, AddRef), asCALL_THISCALL);

		/*
		!CompatMeta
		for     Net
		with    void $beh6()
		as      void $beh6()
		being   behavior
		of      HttpRequest
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		Engine->RegisterObjectBehaviour("HttpRequest", asBEHAVE_RELEASE, "void f()", asMETHOD(HttpRequest, RemoveRef), asCALL_THISCALL);
		Engine->SetDefaultNamespace("");

		RegisterAll(Engine, s_BaseExports);
		RegisterAll(Engine, s_Exports);
	}

#undef C
}