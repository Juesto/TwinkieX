#include "pch.h"
#include "MemoryBuffer.h"

MemoryBuffer::MemoryBuffer(const void* Data, uint64_t size)
{
    uint8_t* NewData = (uint8_t*)malloc(size);
    memcpy_s(NewData, size, Data, size);
    this->Data = NewData;
    Size = size;
}

MemoryBuffer::MemoryBuffer(uint64_t size, uint8_t init)
{
    uint8_t* NewData = (uint8_t*)malloc(size);
    memset(NewData, init, size);
}

MemoryBuffer::~MemoryBuffer()
{
    free(Data);
}

void MemoryBuffer::AddRef()
{
    RefCount++;
}

void MemoryBuffer::RemoveRef()
{
    if (!--RefCount) delete this;
}

void MemoryBuffer::Resize(uint64_t size)
{
    uint8_t* OldData = Data;
    uint8_t* NewData = (uint8_t*)malloc(size);

    memcpy_s(NewData, size, OldData, Size);

    Data = NewData;
    Size = size;
    // Not a bug, this       vvvv   is correct behaviour
    Cursor = Cursor > Size ? Size : Cursor;

    free(OldData);
}

uint64_t MemoryBuffer::GetSize() const
{
    return Size;
}

uint64_t MemoryBuffer::GetPosition() const
{
    return Cursor;
}

void MemoryBuffer::Seek(uint64_t pos, SeekingDirection dir)
{
    switch (dir)
    {
    case SeekingDirection::FromStart:
        Cursor = pos;
        break;
    case SeekingDirection::Forwards:
        Cursor += pos;
        break;
    case SeekingDirection::Backwards:
        if (pos <= Cursor)
            Cursor -= pos;
        else
            Cursor = 0;
        break;
    }

    Cursor = Cursor > Size ? Size : Cursor;
}

bool MemoryBuffer::AtEnd() const
{
    return Size ? Cursor >= Size : true;
}

template<typename T>
using MakeP = T*;

#define MemoryBuffer_WriteFn(T) void MemoryBuffer::Write(const T& i) \
{ \
    if (sizeof(decltype(i)) > Size - Cursor) return; \
    *(MakeP<std::remove_cvref_t<decltype(i)>>)(Data + Cursor) = i; \
    Cursor += sizeof(decltype(i)); \
}

MemoryBuffer_WriteFn(uint8_t);
MemoryBuffer_WriteFn(uint16_t);
MemoryBuffer_WriteFn(uint32_t);
MemoryBuffer_WriteFn(uint64_t);
MemoryBuffer_WriteFn(int8_t);
MemoryBuffer_WriteFn(int16_t);
MemoryBuffer_WriteFn(int32_t);
MemoryBuffer_WriteFn(int64_t);
MemoryBuffer_WriteFn(float);
MemoryBuffer_WriteFn(double);

void MemoryBuffer::Write(const std::u8string& str)
{
    if (str.size() > Size - Cursor) return;
    memcpy_s(Data + Cursor, Size - Cursor, str.data(), str.size());
}

void MemoryBuffer::Write(const MemoryBuffer& buf, uint64_t size)
{
    if (size > Size - Cursor) return;
    memcpy_s(Data + Cursor, Size - Cursor, buf.Data, size);
}

// TODO: Rewrite.
void MemoryBuffer::WriteFromBase64(const std::u8string& str, bool url)
{
    if (str.empty()) return;

    const char* Alphabet = url ? "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"
        : "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    auto GetVal = [Alphabet](char8_t c) -> int {
        if (c == '=') return -2;
        for (int i = 0; i < 64; ++i) {
            if (Alphabet[i] == c) return i;
        }
        return -1;
        };

    size_t Len = str.size();
    if (Len % 4 != 0) return;

    size_t Padding = 0;
    if (Len > 0 && str[Len - 1] == '=') Padding++;
    if (Len > 1 && str[Len - 2] == '=') Padding++;

    uint64_t DecodedSize = (Len / 4) * 3 - Padding;
    if (DecodedSize > Size - Cursor) return;

    std::vector<uint8_t> Temp;
    Temp.reserve(DecodedSize);

    for (size_t i = 0; i < Len; i += 4) {
        int V0 = GetVal(str[i]);
        int V1 = GetVal(str[i + 1]);
        int V2 = GetVal(str[i + 2]);
        int V3 = GetVal(str[i + 3]);

        if (V0 < 0 || V1 < 0 || (V2 < -1) || (V3 < -1)) return;

        uint32_t Triple = (V0 << 18) | (V1 << 12) | ((V2 == -2 ? 0 : V2) << 6) | (V3 == -2 ? 0 : V3);

        Temp.push_back((uint8_t)((Triple >> 16) & 0xFF));
        if (V2 != -2) {
            Temp.push_back((uint8_t)((Triple >> 8) & 0xFF));
        }
        if (V3 != -2) {
            Temp.push_back((uint8_t)(Triple & 0xFF));
        }
    }

    if (Temp.size() != DecodedSize) return;
    memcpy_s(Data + Cursor, Size - Cursor, Temp.data(), DecodedSize);
    Cursor += DecodedSize;
}

static int HexCharToInt(char8_t c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

void MemoryBuffer::WriteFromHex(const std::u8string& str)
{
    if (str.size() % 2 != 0) return;

    uint64_t ByteCount = str.size() / 2;
    if (ByteCount > Size - Cursor) return;

    for (size_t i = 0; i < str.size(); ++i)
    {
        if (HexCharToInt(str[i]) == -1) return;
    }

    for (size_t i = 0; i < str.size(); i += 2)
    {
        int High = HexCharToInt(str[i]);
        int Low = HexCharToInt(str[i + 1]);
        Data[Cursor++] = (uint8_t)((High << 4) | Low);
    }
}

#define MemoryBuffer_ReadFn(T, Name) T MemoryBuffer::Read##Name() \
{ \
    if (sizeof(T) > Size - Cursor) return T{};\
    Cursor += sizeof(T); \
    return *(T*)(Data + Cursor); \
}

MemoryBuffer_ReadFn(uint8_t, UInt8);
MemoryBuffer_ReadFn(uint16_t, UInt16);
MemoryBuffer_ReadFn(uint32_t, UInt32);
MemoryBuffer_ReadFn(uint64_t, UInt64);
MemoryBuffer_ReadFn(int8_t, Int8);
MemoryBuffer_ReadFn(int16_t, Int16);
MemoryBuffer_ReadFn(int32_t, Int32);
MemoryBuffer_ReadFn(int64_t, Int64);
MemoryBuffer_ReadFn(float, Float);
MemoryBuffer_ReadFn(double, Double);

std::u8string MemoryBuffer::ReadString(uint64_t size)
{
    return std::u8string((char8_t*)(Data + Cursor), size);
}

MemoryBuffer MemoryBuffer::ReadBuffer(uint64_t size)
{
    return MemoryBuffer(Data, size);
}

std::u8string MemoryBuffer::ReadToBase64(uint64_t size, bool url)
{
    if (size > Size - Cursor) size = Size - Cursor;
    if (size == 0) return {};

    const char* Alphabet = url ? "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_" : "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::u8string Result;
    Result.reserve(((size + 2) / 3) * 4);

    uint64_t I = 0;
    while (I < size) {
        uint32_t OctetA = I < size ? Data[Cursor + I++] : 0;
        uint32_t OctetB = I < size ? Data[Cursor + I++] : 0;
        uint32_t OctetC = I < size ? Data[Cursor + I++] : 0;

        uint32_t Triple = (OctetA << 16) + (OctetB << 8) + OctetC;

        Result.push_back(Alphabet[(Triple >> 18) & 0x3F]);
        Result.push_back(Alphabet[(Triple >> 12) & 0x3F]);
        Result.push_back(I > size + 1 ? '=' : Alphabet[(Triple >> 6) & 0x3F]);
        Result.push_back(I > size ? '=' : Alphabet[Triple & 0x3F]);
    }

    Cursor += size;
    return Result;
}

std::u8string MemoryBuffer::ReadToHex(uint64_t size, bool upper)
{
    if (size > Size - Cursor) size = Size - Cursor;

    const char* HexDigits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    std::u8string Result;
    Result.reserve(size * 2);

    for (uint64_t I = 0; I < size; ++I)
    {
        uint8_t B = Data[Cursor + I];
        Result.push_back(HexDigits[B >> 4]);
        Result.push_back(HexDigits[B & 0x0F]);
    }

    Cursor += size;
    return Result;
}