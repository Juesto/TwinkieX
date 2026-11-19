#pragma once
#include "pch.h"

namespace Gecko::Exports::Global
{
	void Registrar(asIScriptEngine*);
}

enum class SeekingDirection : int32_t
{
	FromStart,
	Forwards,
	Backwards = -1
};

class MemoryBuffer
{
	uint8_t* Data = nullptr;
	size_t Size = 0;
	mutable size_t Cursor = 0;
	mutable size_t RefCount = 0;

	friend void Gecko::Exports::Global::Registrar(asIScriptEngine*);

	void AddRef();
	void RemoveRef();

public:
	MemoryBuffer() = default;
	~MemoryBuffer();
	MemoryBuffer(const void* Data, uint64_t Size);
	MemoryBuffer(uint64_t size, uint8_t init = 0);

	MemoryBuffer& operator=(const MemoryBuffer&) = default;

	void Resize(uint64_t size);
	uint64_t GetSize() const;
	uint64_t GetPosition() const;
	void Seek(uint64_t pos, SeekingDirection dir = SeekingDirection::FromStart);
	bool AtEnd() const;

	void Write(const uint8_t& i);
	void Write(const uint16_t& i);
	void Write(const uint32_t& i);
	void Write(const uint64_t& i);
	void Write(const int8_t& i);
	void Write(const int16_t& i);
	void Write(const int32_t& i);
	void Write(const int64_t& i);
	void Write(const float& i);
	void Write(const double& i);
	void Write(const std::u8string& str);
	void Write(const MemoryBuffer& buf, uint64_t size);
	void WriteFromBase64(const std::u8string&, bool url = false);
	void WriteFromHex(const std::u8string&);

	uint8_t ReadUInt8();
	uint16_t ReadUInt16();
	uint32_t ReadUInt32();
	uint64_t ReadUInt64();
	int8_t ReadInt8();
	int16_t ReadInt16();
	int32_t ReadInt32();
	int64_t ReadInt64();
	float ReadFloat();
	double ReadDouble();
	std::u8string ReadString(uint64_t size);
	MemoryBuffer ReadBuffer(uint64_t size);
	std::u8string ReadToBase64(uint64_t size, bool url = false);
	std::u8string ReadToHex(uint64_t size, bool upper = false);
};