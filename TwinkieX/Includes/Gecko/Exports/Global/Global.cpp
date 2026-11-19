
#include "pch.h"
#include "Global.h"
#include <Gecko/Exports/GeckoAutoExport.h>
#include "MemoryBuffer/MemoryBuffer.h"

namespace Gecko::Exports::Global
{
	static std::vector<GeckoExport> s_Exports;
#define GLOBAL_EXPORT(Decl_, Fn_) GECKO_EXPORT_FN(s_Exports, "", Decl_, Fn_)
#define GLOBAL_MEMORYBUFFER_METHOD(Decl_, Ptr_, Conv_)          GECKO_EXPORT_METHOD(s_Exports, "", "MemoryBuffer", Decl_, Ptr_, Conv_)
#define GLOBAL_MEMORYBUFFER_BEHAVIOUR(Beh_, Decl_, Ptr_, Conv_) GECKO_EXPORT_BEHAVIOUR(s_Exports, "", "MemoryBuffer", Beh_, Decl_, Ptr_, Conv_)

	/*
	!CompatMeta
	for     Global
	with    void yield()
	as      void yield()
	being   function
	declare partial
	reason  Does not raise error when called in non-main context.
	since   v0.1.0
	!CompatMeta
	*/
	void yield()
	{
		auto Main = TheScriptManager->GetActiveScript()->Ctxs.CtxMain;
		if (Main && TheScriptManager->ActiveCallbackType == CallbackType::Main) Main->Suspend();
		else if (TheScriptManager->ActiveCallbackType != CallbackType::Main) (void)0; // TODO: raise an error
	}
	GLOBAL_EXPORT("void yield()", yield);

	/*
	!CompatMeta
	for     Global
	with    void yield(uint frames)
	as      void yield(uint frames)
	being   function
	declare partial
	reason  Does not raise error when called in non-main context.
	since   v0.1.0
	!CompatMeta
	*/
	void yieldFor(uint32_t For)
	{
		TheScriptManager->GetActiveScript()->YieldFor = For ? For - 1 : 0;
		if (For) yield();
	}
	GLOBAL_EXPORT("void yield(uint)", yieldFor);

	/*
	!CompatMeta
	for     Global
	with    void trace(const string&in)
	as      void trace(const string&in)
	being   function
	declare partial
	reason  Did not implement generic yet (where first arg is anything)
	!CompatMeta
	*/
	void trace(std::u8string& Str)
	{
		LoggertAngelscript.trace((char8_t*)std::format("[{}] {}", (char*)Gecko::TheScriptManager->GetActiveScript()->Id.c_str(), (char*)Str.c_str()).c_str());
	}
	GLOBAL_EXPORT("void trace(const string &in)", trace);

	/*
	!CompatMeta
	for     Global
	with    void print(const string&in)
	as      void print(const string&in)
	being   function
	declare partial
	reason  Did not implement generic yet (where first arg is anything)
	!CompatMeta
	*/
	void print(std::u8string& Str)
	{
		LoggertAngelscript.print((char8_t*)std::format("[{}] {}", (char*)Gecko::TheScriptManager->GetActiveScript()->Id.c_str(), (char*)Str.c_str()).c_str());
	}
	GLOBAL_EXPORT("void print(const string &in)", print);

	/*
	!CompatMeta
	for     Global
	with    void warn(const string&in)
	as      void warn(const string&in)
	being   function
	declare partial
	reason  Did not implement generic yet (where first arg is anything)
	!CompatMeta
	*/
	void warn(std::u8string& Str)
	{
		LoggertAngelscript.warn((char8_t*)std::format("[{}] {}", (char*)Gecko::TheScriptManager->GetActiveScript()->Id.c_str(), (char*)Str.c_str()).c_str());
	}
	GLOBAL_EXPORT("void warn(const string &in)", warn);

	/*
	!CompatMeta
	for     Global
	with    void error(const string&in)
	as      void error(const string&in)
	being   function
	declare partial
	reason  Did not implement generic yet (where first arg is anything)
	!CompatMeta
	*/
	void error(std::u8string& Str)
	{
		LoggertAngelscript.error((char8_t*)std::format("[{}] {}", (char*)Gecko::TheScriptManager->GetActiveScript()->Id.c_str(), (char*)Str.c_str()).c_str());
	}
	GLOBAL_EXPORT("void error(const string &in)", error);

	static void CopyMemoryBuffer(MemoryBuffer& Dst, const MemoryBuffer& Src)
	{
		if (&Dst == &Src) return;
		Dst.Resize(Src.GetSize());
		Dst.Seek(0);
		Dst.Write(Src, Src.GetSize());
		Dst.Seek(Src.GetPosition());
	}

	/*
	!CompatMeta
	for     Global
	with    MemoryBuffer@ MemoryBuffer()
	as      MemoryBuffer@ MemoryBuffer()
	being   behavior
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	static MemoryBuffer* MemoryBuffer_Factory()
	{
		return new MemoryBuffer();
	}
	GLOBAL_MEMORYBUFFER_BEHAVIOUR(asBEHAVE_FACTORY, "MemoryBuffer@+ f()", asFUNCTION(MemoryBuffer_Factory), asCALL_CDECL);

	/*
	!CompatMeta
	for     Global
	with    MemoryBuffer@ MemoryBuffer(const MemoryBuffer&in)
	as      MemoryBuffer@ MemoryBuffer(const MemoryBuffer&in)
	being   behavior
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	static MemoryBuffer* MemoryBuffer_FactoryCopy(const MemoryBuffer& Other)
	{
		MemoryBuffer* Buf = new MemoryBuffer();
		CopyMemoryBuffer(*Buf, Other);
		return Buf;
	}
	GLOBAL_MEMORYBUFFER_BEHAVIOUR(asBEHAVE_FACTORY, "MemoryBuffer@+ f(const MemoryBuffer&in)", asFUNCTION(MemoryBuffer_FactoryCopy), asCALL_CDECL);

	/*
	!CompatMeta
	for     Global
	with    MemoryBuffer@ MemoryBuffer(uint64 size)
	as      MemoryBuffer@ MemoryBuffer(uint64 size)
	being   behavior
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	static MemoryBuffer* MemoryBuffer_FactorySize(uint64_t Size)
	{
		return new MemoryBuffer(Size);
	}
	GLOBAL_MEMORYBUFFER_BEHAVIOUR(asBEHAVE_FACTORY, "MemoryBuffer@+ f(uint64 size)", asFUNCTION(MemoryBuffer_FactorySize), asCALL_CDECL);

	/*
	!CompatMeta
	for     Global
	with    MemoryBuffer@ MemoryBuffer(uint64 size, uint8 init)
	as      MemoryBuffer@ MemoryBuffer(uint64 size, uint8 init)
	being   behavior
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	static MemoryBuffer* MemoryBuffer_FactorySizeInit(uint64_t Size, uint8_t Init)
	{
		return new MemoryBuffer(Size, Init);
	}
	GLOBAL_MEMORYBUFFER_BEHAVIOUR(asBEHAVE_FACTORY, "MemoryBuffer@+ f(uint64 size, uint8 init)", asFUNCTION(MemoryBuffer_FactorySizeInit), asCALL_CDECL);

	/*
	!CompatMeta
	for     Global
	with    MemoryBuffer& opAssign(const MemoryBuffer&in)
	as      MemoryBuffer& opAssign(const MemoryBuffer&in)
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	static MemoryBuffer& MemoryBuffer_opAssign(const MemoryBuffer& Other, MemoryBuffer* Buf)
	{
		CopyMemoryBuffer(*Buf, Other);
		return *Buf;
	}
	GLOBAL_MEMORYBUFFER_METHOD("MemoryBuffer& opAssign(const MemoryBuffer&in)", asFUNCTION(MemoryBuffer_opAssign), asCALL_CDECL_OBJLAST);

	/*
	!CompatMeta
	for     Global
	with    void Resize(uint64 size)
	as      void Resize(uint64 size)
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	GLOBAL_MEMORYBUFFER_METHOD("void Resize(uint64 size)", asMETHOD(MemoryBuffer, Resize), asCALL_THISCALL);

	/*
	!CompatMeta
	for     Global
	with    uint64 GetSize() const
	as      uint64 GetSize() const
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	GLOBAL_MEMORYBUFFER_METHOD("uint64 GetSize()", asMETHOD(MemoryBuffer, GetSize), asCALL_THISCALL);

	/*
	!CompatMeta
	for     Global
	with    uint64 GetPosition() const
	as      uint64 GetPosition() const
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	GLOBAL_MEMORYBUFFER_METHOD("uint64 GetPosition()", asMETHOD(MemoryBuffer, GetPosition), asCALL_THISCALL);

	/*
	!CompatMeta
	for     Global
	with    void Seek(uint64 pos, int dir = 0)
	as      void Seek(uint64 pos, int dir = 0)
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	static void MemoryBuffer_Seek(uint64_t Pos, int Dir, MemoryBuffer* Buf)
	{
		Buf->Seek(Pos, (SeekingDirection)Dir);
	}
	GLOBAL_MEMORYBUFFER_METHOD("void Seek(uint64 pos, int dir = 0)", asFUNCTION(MemoryBuffer_Seek), asCALL_CDECL_OBJLAST);

	/*
	!CompatMeta
	for     Global
	with    bool AtEnd() const
	as      bool AtEnd() const
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	GLOBAL_MEMORYBUFFER_METHOD("bool AtEnd()", asMETHOD(MemoryBuffer, AtEnd), asCALL_THISCALL);

	/*
	!CompatMeta
	for     Global
	with    void Write(const uint8&in i)
	as      void Write(const uint8&in i)
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	GLOBAL_MEMORYBUFFER_METHOD("void Write(const uint8&in i)", asMETHODPR(MemoryBuffer, Write, (const uint8_t&), void), asCALL_THISCALL);

	/*
	!CompatMeta
	for     Global
	with    void Write(const uint16&in i)
	as      void Write(const uint16&in i)
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	GLOBAL_MEMORYBUFFER_METHOD("void Write(const uint16&in i)", asMETHODPR(MemoryBuffer, Write, (const uint16_t&), void), asCALL_THISCALL);

	/*
	!CompatMeta
	for     Global
	with    void Write(const uint&in i)
	as      void Write(const uint&in i)
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	GLOBAL_MEMORYBUFFER_METHOD("void Write(const uint&in i)", asMETHODPR(MemoryBuffer, Write, (const uint32_t&), void), asCALL_THISCALL);

	/*
	!CompatMeta
	for     Global
	with    void Write(const uint64&in i)
	as      void Write(const uint64&in i)
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	GLOBAL_MEMORYBUFFER_METHOD("void Write(const uint64&in i)", asMETHODPR(MemoryBuffer, Write, (const uint64_t&), void), asCALL_THISCALL);

	/*
	!CompatMeta
	for     Global
	with    void Write(const int8&in i)
	as      void Write(const int8&in i)
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	GLOBAL_MEMORYBUFFER_METHOD("void Write(const int8&in i)", asMETHODPR(MemoryBuffer, Write, (const int8_t&), void), asCALL_THISCALL);

	/*
	!CompatMeta
	for     Global
	with    void Write(const int16&in i)
	as      void Write(const int16&in i)
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	GLOBAL_MEMORYBUFFER_METHOD("void Write(const int16&in i)", asMETHODPR(MemoryBuffer, Write, (const int16_t&), void), asCALL_THISCALL);

	/*
	!CompatMeta
	for     Global
	with    void Write(const int&in i)
	as      void Write(const int&in i)
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	GLOBAL_MEMORYBUFFER_METHOD("void Write(const int&in i)", asMETHODPR(MemoryBuffer, Write, (const int32_t&), void), asCALL_THISCALL);

	/*
	!CompatMeta
	for     Global
	with    void Write(const int64&in i)
	as      void Write(const int64&in i)
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	GLOBAL_MEMORYBUFFER_METHOD("void Write(const int64&in i)", asMETHODPR(MemoryBuffer, Write, (const int64_t&), void), asCALL_THISCALL);

	/*
	!CompatMeta
	for     Global
	with    void Write(const float&in i)
	as      void Write(const float&in i)
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	GLOBAL_MEMORYBUFFER_METHOD("void Write(const float&in i)", asMETHODPR(MemoryBuffer, Write, (const float&), void), asCALL_THISCALL);

	/*
	!CompatMeta
	for     Global
	with    void Write(const double&in i)
	as      void Write(const double&in i)
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	GLOBAL_MEMORYBUFFER_METHOD("void Write(const double&in i)", asMETHODPR(MemoryBuffer, Write, (const double&), void), asCALL_THISCALL);

	/*
	!CompatMeta
	for     Global
	with    void Write(const string&in str)
	as      void Write(const string&in str)
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	GLOBAL_MEMORYBUFFER_METHOD("void Write(const string&in str)", asMETHODPR(MemoryBuffer, Write, (const std::u8string&), void), asCALL_THISCALL);

	/*
	!CompatMeta
	for     Global
	with    void WriteFromBuffer(const MemoryBuffer&in buf, uint64 size)
	as      void WriteFromBuffer(const MemoryBuffer&in buf, uint64 size)
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	GLOBAL_MEMORYBUFFER_METHOD("void WriteFromBuffer(const MemoryBuffer&in buf, uint64 size)", asMETHODPR(MemoryBuffer, Write, (const MemoryBuffer&, uint64_t), void), asCALL_THISCALL);

	/*
	!CompatMeta
	for     Global
	with    uint8 ReadUInt8()
	as      uint8 ReadUInt8()
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	GLOBAL_MEMORYBUFFER_METHOD("uint8 ReadUInt8()", asMETHOD(MemoryBuffer, ReadUInt8), asCALL_THISCALL);

	/*
	!CompatMeta
	for     Global
	with    uint16 ReadUInt16()
	as      uint16 ReadUInt16()
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	GLOBAL_MEMORYBUFFER_METHOD("uint16 ReadUInt16()", asMETHOD(MemoryBuffer, ReadUInt16), asCALL_THISCALL);

	/*
	!CompatMeta
	for     Global
	with    uint ReadUInt32()
	as      uint ReadUInt32()
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	GLOBAL_MEMORYBUFFER_METHOD("uint ReadUInt32()", asMETHOD(MemoryBuffer, ReadUInt32), asCALL_THISCALL);

	/*
	!CompatMeta
	for     Global
	with    uint64 ReadUInt64()
	as      uint64 ReadUInt64()
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	GLOBAL_MEMORYBUFFER_METHOD("uint64 ReadUInt64()", asMETHOD(MemoryBuffer, ReadUInt64), asCALL_THISCALL);

	/*
	!CompatMeta
	for     Global
	with    int8 ReadInt8()
	as      int8 ReadInt8()
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	GLOBAL_MEMORYBUFFER_METHOD("int8 ReadInt8()", asMETHOD(MemoryBuffer, ReadInt8), asCALL_THISCALL);

	/*
	!CompatMeta
	for     Global
	with    int16 ReadInt16()
	as      int16 ReadInt16()
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	GLOBAL_MEMORYBUFFER_METHOD("int16 ReadInt16()", asMETHOD(MemoryBuffer, ReadInt16), asCALL_THISCALL);

	/*
	!CompatMeta
	for     Global
	with    int ReadInt32()
	as      int ReadInt32()
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	GLOBAL_MEMORYBUFFER_METHOD("int ReadInt32()", asMETHOD(MemoryBuffer, ReadInt32), asCALL_THISCALL);

	/*
	!CompatMeta
	for     Global
	with    int64 ReadInt64()
	as      int64 ReadInt64()
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	GLOBAL_MEMORYBUFFER_METHOD("int64 ReadInt64()", asMETHOD(MemoryBuffer, ReadInt64), asCALL_THISCALL);

	/*
	!CompatMeta
	for     Global
	with    float ReadFloat()
	as      float ReadFloat()
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	GLOBAL_MEMORYBUFFER_METHOD("float ReadFloat()", asMETHOD(MemoryBuffer, ReadFloat), asCALL_THISCALL);

	/*
	!CompatMeta
	for     Global
	with    double ReadDouble()
	as      double ReadDouble()
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	GLOBAL_MEMORYBUFFER_METHOD("double ReadDouble()", asMETHOD(MemoryBuffer, ReadDouble), asCALL_THISCALL);

	/*
	!CompatMeta
	for     Global
	with    string ReadString(uint64 size)
	as      string ReadString(uint64 size)
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	GLOBAL_MEMORYBUFFER_METHOD("string ReadString(uint64 size)", asMETHOD(MemoryBuffer, ReadString), asCALL_THISCALL);

	/*
	!CompatMeta
	for     Global
	with    MemoryBuffer@ ReadBuffer(uint64 size)
	as      MemoryBuffer@ ReadBuffer(uint64 size)
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	static MemoryBuffer* MemoryBuffer_ReadBuffer(uint64_t Size, MemoryBuffer* Buf)
	{
		return new MemoryBuffer(Buf->ReadBuffer(Size));
	}
	GLOBAL_MEMORYBUFFER_METHOD("MemoryBuffer@+ ReadBuffer(uint64 size)", asFUNCTION(MemoryBuffer_ReadBuffer), asCALL_CDECL_OBJLAST);

	/*
	!CompatMeta
	for     Global
	with    void WriteFromBase64(const string&in, bool url = false)
	as      void WriteFromBase64(const string&in, bool url = false)
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	GLOBAL_MEMORYBUFFER_METHOD("void WriteFromBase64(const string&in, bool url = false)", asMETHOD(MemoryBuffer, WriteFromBase64), asCALL_THISCALL);

	/*
	!CompatMeta
	for     Global
	with    string ReadToBase64(uint64 size, bool url = false)
	as      string ReadToBase64(uint64 size, bool url = false)
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	GLOBAL_MEMORYBUFFER_METHOD("string ReadToBase64(uint64 size, bool url = false)", asMETHOD(MemoryBuffer, ReadToBase64), asCALL_THISCALL);

	/*
	!CompatMeta
	for     Global
	with    void WriteFromHex(const string&in)
	as      void WriteFromHex(const string&in)
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	GLOBAL_MEMORYBUFFER_METHOD("void WriteFromHex(const string&in)", asMETHOD(MemoryBuffer, WriteFromHex), asCALL_THISCALL);

	/*
	!CompatMeta
	for     Global
	with    string ReadToHex(uint64 size, bool upper = false)
	as      string ReadToHex(uint64 size, bool upper = false)
	being   method
	of      MemoryBuffer
	declare full
	since   v0.1.1
	!CompatMeta
	*/
	GLOBAL_MEMORYBUFFER_METHOD("string ReadToHex(uint64 size, bool upper = false)", asMETHOD(MemoryBuffer, ReadToHex), asCALL_THISCALL);

	void Registrar(asIScriptEngine* Engine)
	{
		RegisterStdString(Engine);
		RegisterScriptArray(Engine, true);

		Engine->RegisterObjectType("MemoryBuffer", sizeof(MemoryBuffer), asOBJ_REF);

		/*
		!CompatMeta
		for     Global
		with    void $beh5()
		as      void $beh5()
		being   behavior
		of      MemoryBuffer
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		Engine->RegisterObjectBehaviour("MemoryBuffer", asBEHAVE_ADDREF, "void f()", asMETHOD(MemoryBuffer, AddRef), asCALL_THISCALL);

		/*
		!CompatMeta
		for     Global
		with    void $beh6()
		as      void $beh6()
		being   behavior
		of      MemoryBuffer
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		Engine->RegisterObjectBehaviour("MemoryBuffer", asBEHAVE_RELEASE, "void f()", asMETHOD(MemoryBuffer, RemoveRef), asCALL_THISCALL);

		RegisterAll(Engine, s_Exports);
	}
}
