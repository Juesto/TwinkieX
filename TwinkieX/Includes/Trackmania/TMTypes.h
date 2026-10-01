#include <Utils/Offsets.h>
#pragma once

class CMwNod;

using ActionFn = void(
#ifdef X86
	__thiscall
#endif
	*)(CMwNod*);

// Resizable array type used by the game
template <typename T>
struct CFastBuffer
{
#ifdef TMCN
	T* Ptr = nullptr;
	uint32_t Size = 0;
	uint32_t Capacity = 0;
#elif defined(GAMEBOX)
	uint32_t Size = 0;
	T* Ptr = nullptr;
	uint32_t Capacity = 0;
#endif

	T* operator[](size_t Idx)
	{
		return Ptr + Idx;
	}

	T* begin() { return Ptr; }
	T* end() { return Ptr + Size; }

	const T* begin() const { return Ptr; }
	const T* end() const { return Ptr + Size; }
};

// Single-size array type used by the game
template <typename T>
struct CFastArray
{
#ifdef MANIAPLANET
	T* Ptr = nullptr;
	uint32_t Size = 0;
#else
	uintptr_t vf;
	uint32_t Size = 0;
	T* Ptr = nullptr;
#endif


	T* begin() { return Ptr; }
	T* end() { return Ptr + Size; }

	const T* begin() const { return Ptr; }
	const T* end() const { return Ptr + Size; }
};

// Used for parameters to PROCs
struct CMwParam 
{
	void* Ptr;
};

#ifdef TMCN
// Information about a class' member
// This class is exclusive to TMCN
struct CMwMemberInfo
{
	// ARRAYBASE => https://openplanet.dev/docs/api/global/MwSArray
	// [TYPE] and [TYPE]ALT are DIFFERENT. Do NOT ignore them.
	enum eType
	{
		ACTION = 0,

		BOOL = 1,
		BOOLARRAY = 2, // EXTRAPOLATED
		BOOLBUFFER = 3, // EXTRAPOLATED
		BOOLBUFFERCAT = 4, // EXTRAPOLATED
		BOOLARRAYBASE = 5, // EXTRAPOLATED, https://openplanet.dev/docs/api/global/MwSArray

		CLASS = 10,
		CLASSARRAY = 11,
		CLASSBUFFER = 12,
		CLASSBUFFERCAT = 13, // EXTRAPOLATED
		CLASSARRAYBASE = 14, // https://openplanet.dev/docs/api/global/MwSArray

		COLOR = 19, // EXTRAPOLATED
		COLORARRAY = 20,
		COLORBUFFER = 21, // EXTRAPOLATED
		COLORBUFFERCAT = 22,
		COLORARRAYBASE = 23, // EXTRAPOLATED, https://openplanet.dev/docs/api/global/MwSArray

		ENUM = 28,
		ENUMARRAY = 29,
		ENUMBUFFER = 30,

		INT = 37,
		INTARRAY = 38,
		INTBUFFER = 39, // EXTRAPOLATED
		INTBUFFERCAT = 40, // EXTRAPOLATED
		INTARRAYBASE = 41, // EXTRAPOLATED, https://openplanet.dev/docs/api/global/MwSArray

		INTRANGE = 46,

		ISO4 = 47,

		ID = 65,
		
		NATURAL = 74,
		NATURALARRAY = 75, // EXTRAPOLATED
		NATURALBUFFER = 76, // EXTRAPOLATED
		NATURALBUFFERCAT = 77, // EXTRAPOLATED
		NATURALARRAYBASE = 78, // EXTRAPOLATED, https://openplanet.dev/docs/api/global/MwSArray
		
		REAL = 84,
		REALARRAY = 85,
		REALBUFFER = 86, // EXTRAPOLATED
		REALBUFFERCAT = 87, // EXTRAPOLATED
		REALARRAYBASE = 88, // EXTRAPOLATED, https://openplanet.dev/docs/api/global/MwSArray
		
		REALRANGE = 93,

		STRING = 94,
		STRINGARRAY = 95, // EXTRAPOLATED
		STRINGBUFFER = 96,
		STRINGBUFFERCAT = 97, // EXTRAPOLATED
		STRINGARRAYBASE = 98,  // EXTRAPOLATED, https://openplanet.dev/docs/api/global/MwSArray
		
		STRINGINT = 103,
		STRINGINTARRAY = 104,
		STRINGINTBUFFER = 105,
		STRINGINTBUFFERCAT = 106, // EXTRAPOLATED
		STRINGINTARRAYBASE = 107,  // EXTRAPOLATED, https://openplanet.dev/docs/api/global/MwSArray
		
		VEC2 = 112,
		VEC2ARRAY = 113, // EXTRAPOLATED
		VEC2BUFFER = 114,
		VEC2BUFFERCAT = 115, // EXTRAPOLATED
		VEC2ARRAYBASE = 116, // EXTRAPOLATED, https://openplanet.dev/docs/api/global/MwSArray

		VEC3 = 121,
		
		VEC4 = 132,
		VEC4ARRAY = 133,
		VEC4BUFFER = 134,

		PROC = 184,
		
		CLASSNOTPERSISTENT = 186, // https://next.openplanet.dev/MetaNotPersistent

		CLASSNOTPERSISTENTBUFFER = 197,

		CLASSALT = 240, // App.GameScene.SceneFxMgr

		MAX = 512
	};

	std::string GetName() const;

	enum eFlags 
	{
		GET = 0b00000001,
		SET = 0b00000010,
		ADD = 0b00000100,
		SUB = 0b00001000,
		VIRTUAL_GET = 0b00010000,
		VIRTUAL_SET = 0b00100000,
		VIRTUAL_ADD = 0b01000000,
		VIRTUAL_SUB = 0b10000000,
	};

	// TODO: Fully reverse this enum type
	// Type of the member
	eType MemberType;

	// Member ID, unique between classes
	uint32_t MemberID;

	// I have no idea
	CMwParam* pParam;

	// Offset of the member in the class instance, -1 for methods and virtual members
	uint32_t MemberOffset;

	uint32_t Pad0;

	// Name of the member
	const char* MemberName;

	// Unknown pointer
	uintptr_t p;

	// Flags for the member, see (eFlags)
	uint32_t MemberFlags;
	// Secondary flags for the member, see (eFlags)
	uint32_t MemberFlags2;
};


// Information about a class
// This class is exclusive to TMCN
struct CMwClassInfo
{
	uintptr_t* vf;

	// Class name
	const char* ClassName;

	uint32_t unknown0;
	uint32_t unknown1;

	// Class ID
	uint32_t ClassID;

	uint32_t unknown2;

	// Parent class' info, null if none
	CMwClassInfo* ParentClassInfo;

	// Next class info, usually has nothing to do with this class
	CMwClassInfo* NextClassInfo;
	// Pointer to the class' constructor
	ActionFn CtorFn;

	// Padding for unknown data
	uint64_t Padding[18];

	// File extension for this class
	char* FileExtName;

	// List of member infos
	CMwMemberInfo** Members;
	// Number of member infos in the list
	uint32_t MembersAmount;

	// Iterator function for the beginning of the members array
	CMwMemberInfo** begin();
	// Iterator function for the end of the members array
	CMwMemberInfo** end();

	// Const iterator function for the beginning of the members array
	CMwMemberInfo** begin() const;
	// Const iterator function for the end of the members array
	CMwMemberInfo** end() const;

	std::string GetName() const;

	uint32_t Pad0;
};

// Info of a CLASS type member
struct CMwMemberInfoClass : CMwMemberInfo
{
	ActionFn InstantiationFn;
	CMwClassInfo* ClassInfo;
	uint32_t ClassID;
	uint32_t Pad;
};

struct CMwMemberInfoRealRange : CMwMemberInfo
{
	uint8_t Padding[8];

	float ValueMin;
	float ValueMax;
};

struct CMwMemberInfoIntRange : CMwMemberInfo
{
	uint8_t Padding[8];

	// KEEP THESE AS int

	int ValueMin;
	int ValueMax;
};

struct CMwMemberInfoEnum : CMwMemberInfo
{
	uint8_t Padding[8];

	const char* EnumTypeName;
	int32_t EnumValueNamesLength;
	uint32_t Padding2;
	const char** EnumValueNames;

	uint64_t Unknown;
};

// Info for a member who is an array of CLASSes (like CLASSBUFFER)
struct CMwMemberInfoClassArray : CMwMemberInfo
{
	uint8_t Padding[16];

	const char* ElementNameSingular; // If member name is something like "Tracks", this would be "Track" instead. Could be null.

	uint64_t Padding2;

	CMwClassInfo* ArrayClassInfo;
};

struct CMwMemberInfoProc : CMwMemberInfo
{
	struct Type
	{
		enum EType
		{
			NORMAL = 1,
			CLASS = 2
		} Type;
		// ClassID in case of CLASS
		uint32_t AdditionalInfo; 
	};

	struct Param
	{
		Type Type;
		const char* Name;
		uint8_t Flags;
	};

	uint64_t Padding;
	void* Ptr3;
	ActionFn MethodItself;
	uint64_t Padding3;

	uint32_t ParamCount;
	uint32_t Padding2;
	Type* Params;
	const char** ParamNames;
	uint8_t* ParamFlags;

	std::vector<Param> GetParams()
	{
		std::vector<Param> OutParams;

		for (size_t ParamIdx = 0; ParamIdx < ParamCount; ParamIdx++)
		{
			OutParams.push_back(Param{ this->Params[ParamIdx], ParamNames[ParamIdx], ParamFlags[ParamIdx]});
		}

		return OutParams;
	}
};

struct CFastStringInt
{
	wchar_t* CStr;
	uint32_t Count;
};
// GAMEBOX is for TM1, TMO, TMS, TMSX and ESWC
#elif defined(GAMEBOX)
#ifndef TM1
struct CFastStringInt
{
	uint32_t Count;
	wchar_t* CStr;
};
#else
struct CFastStringInt
{
	uint32_t Pad;
	uint32_t Count;
	char* CStr;
};
#endif

// Information about a class' member
// This class is shared between TM1, TMO, TMS, TMSX and ESWC
struct CMwMemberInfo
{
	enum eType
	{
		ACTION,
		BOOL,
		BOOLARRAY,
		BOOLBUFFER,
		BOOLBUFFERCAT,
		CLASS,
		CLASSARRAY,
		CLASSBUFFER,
		CLASSBUFFERCAT,
		COLOR,
		COLORARRAY,
		COLORBUFFER,
		COLORBUFFERCAT,
		ENUM,
		INT,
		INTARRAY,
		INTBUFFER,
		INTBUFFERCAT,
		INTRANGE,
		ISO4,
		ISO4ARRAY,
		ISO4BUFFER,
		ISO4BUFFERCAT,
		ISO3,
		ISO3ARRAY,
		ISO3BUFFER,
		ISO3BUFFERCAT,
		NATURAL,
		NATURALARRAY,
		NATURALBUFFER,
		NATURALBUFFERCAT,
		NATURALRANGE,
		REAL,
		REALARRAY,
		REALBUFFER,
		REALBUFFERCAT,
		REALRANGE,
		ID,
		IDARRAY,
		IDBUFFER,
		IDBUFFERCAT,
#ifndef TM1
		STRING,
		STRINGARRAY,
		STRINGBUFFER,
		STRINGBUFFERCAT,
		STRINGINT,
		STRINGINTARRAY,
		STRINGINTBUFFER,
		STRINGINTBUFFERCAT,
#endif
		VEC2,
		VEC2ARRAY,
		VEC2BUFFER,
		VEC2BUFFERCAT,
		VEC3,
		VEC3ARRAY,
		VEC3BUFFER,
		VEC3BUFFERCAT,
		VEC4,
		VEC4ARRAY,
		VEC4BUFFER,
		VEC4BUFFERCAT,
		PROC
	};

	enum eFlags
	{
		GET = 0b00000001,
		SET = 0b00000010,
		ADD = 0b00000100,
		SUB = 0b00001000,
		VIRTUAL_GET = 0b00010000,
		VIRTUAL_SET = 0b00100000,
		VIRTUAL_ADD = 0b01000000,
		VIRTUAL_SUB = 0b10000000,
	};

	std::string GetName() const;

	// Type of the member
	eType MemberType;

	// Member ID, unique between classes
	uint32_t MemberID;

	// I have no idea
	CMwParam* pParam;

	// Offset of the member in the class instance, -1 for methods and virtual members
	uint32_t MemberOffset;

	// Member name
	const char* MemberName;

	// Flags for the member, see (eFlags)
	uint32_t MemberFlags;
	// Secondary flags for the member, see (eFlags)
	uint32_t MemberFlags2;
};

// Information about a class' member
// This class is shared between TM1, TMO, TMS, TMSX and ESWC
struct CMwClassInfo
{
	// Virtual function table pointer
	ActionFn* vftable;

	// Class ID
	uint32_t ClassID;

	// Parent class' info, null if none
	CMwClassInfo* ParentClassInfo;

	uint32_t unknown0;
	uint32_t unknown1;
	
	// Class name
	char* ClassName;

	// Next class info, usually has nothing to do with this class
	CMwClassInfo* NextClassInfo;
#ifdef TM1
	uint32_t unknown2;
#endif
	// Pointer to the class' constructor
	ActionFn CtorFn;

	// List of member infos
	CMwMemberInfo** Members;
	// Number of member infos in the list
	uint32_t MembersAmount;

	// Iterator function for the beginning of the members array
	CMwMemberInfo** begin();
	// Iterator function for the end of the members array
	CMwMemberInfo** end();

	// Const iterator function for the beginning of the members array
	CMwMemberInfo** begin() const;
	// Const iterator function for the end of the members array
	CMwMemberInfo** end() const;

	std::string GetName() const;
};


// Info of a CLASS type member
struct CMwMemberInfoClass : CMwMemberInfo
{
#ifndef TMU
	uint32_t Pad0;
#else
	CMwClassInfo* ClassInfo;
#endif

	uint32_t Pad1;

#ifndef TMU
	CMwClassInfo* ClassInfo;
#else
	uint32_t Pad0;
#endif

	uint32_t Pad2;
};

// Info for a member who is an array of CLASSes (like CLASSBUFFER)
struct CMwMemberInfoClassArray : CMwMemberInfo
{
	uint32_t Pad0;

	uint32_t Pad1;

	uint32_t Pad2;

#ifndef TMU
	const char* ElementNameSingular;
#else
	CMwClassInfo* ArrayClassInfo;
#endif

	uint32_t Pad3;

#ifndef TMU
	CMwClassInfo* ArrayClassInfo;
#else
	uint32_t Pad4;
#endif
};
#endif

// Info of an ACTION type member
struct CMwMemberInfoAction : CMwMemberInfo
{
	uint8_t Padding[8];

	// The action function itself
	ActionFn Action;
};

// The class used by the app's input port (CTrackMania.InputPort)
struct CInputPort
{
	uint8_t Padding[O_M_CINPUTPORT_CONNECTEDDEVICES];
	CFastArray<CMwNod*> ConnectedDevices;
	uint8_t Padding2[O_M_CINPUTPORT_ISFOCUSED - O_M_CINPUTPORT_CONNECTEDDEVICES - sizeof(CFastArray<uintptr_t>)];
	uint32_t IsFocused;
};

class CMwValueStd
{
public:
	void* pValue;
	void* pValue2;
	unsigned char Pad[16];
};

class CMwStack;

// C4100: unreferenced parameter for function
#pragma warning(disable : 4100)
#pragma warning(push)

// The base class for all game objects
class CMwNod
{
public:
#ifdef MANIAPLANET
	virtual ~CMwNod(void) {}
	virtual void VTablePadding1(void) {}
	virtual CMwClassInfo* MwGetClassInfo(void) { return nullptr; }
	virtual unsigned long MwGetMwClassId(void) { return 0L; }
	virtual bool MwIsKindOf(unsigned long classID) { return false; }
	virtual uint64_t* MwGetId(void) { return nullptr; }
	virtual void MwSetIdName(char const* psz) {}
	virtual void MwIsKilled(CMwNod*) {}
	virtual void MwIsUnreferenced(CMwNod*) {}
	virtual unsigned long VirtualParam_Get(CMwStack* pStack, void* ppValue) { return 0L; }
	virtual unsigned long VirtualParam_Set(CMwStack* pStack, void* pValue)  { return 0L; }
	virtual unsigned long VirtualParam_Add(CMwStack* pStack, void* pValue)  { return 0L; }
	virtual unsigned long VirtualParam_Sub(CMwStack* pStack, void* pValue)  { return 0L; }
#else
	virtual ~CMwNod(void) {}
	virtual CMwClassInfo* MwGetClassInfo(void) { return nullptr; }
	virtual unsigned long MwGetMwClassId(void) { return 0; }
	virtual bool MwIsKindOf(unsigned long classID) { return false; }
	virtual uint64_t* MwGetId(void) { return nullptr; }
	virtual void MwSetIdName(char const* psz) {}
	virtual void VTablePadding6(void) {}
	virtual void MwIsUnreferenced(CMwNod*) {}
	virtual unsigned long VirtualParam_Get(CMwStack* pStack, void* ppValue) { return 0L; }
	virtual unsigned long VirtualParam_Set(CMwStack* pStack, void* pValue)  { return 0L; }
	virtual unsigned long VirtualParam_Add(CMwStack* pStack, void* pValue)  { return 0L; }
	virtual unsigned long VirtualParam_Sub(CMwStack* pStack, void* pValue)  { return 0L; }
#endif

	uint32_t ReferenceCount;
	uint32_t Pad0;
	uintptr_t SystemFid;
	CFastBuffer<CMwNod*>* Dependants;
	uint32_t unknown1;
	uint32_t Pad1;
};

class CMwStack
#ifdef GAMEBOX
	: public CMwNod
#endif
{
public:
#ifdef GAMEBOX
#pragma optimize("", off)
	CMwMemberInfo** ppMemberInfos = nullptr;
	uint32_t m_Size = 0;
	uint32_t m_iCurrentPos = 0;
#pragma optimize("", on)
#elif defined(TMCN)
	enum eItemType
	{
		ITEM_MEMBER = 0,            
		ITEM_BOOL = 0x10000001,
		ITEM_OBJECT = 0x10000002,   
		ITEM_ENUM = 0x10000003,
		ITEM_ISO4 = 0x10000004,
		ITEM_VEC2 = 0x10000005,
		ITEM_VEC3 = 0x10000006,
		ITEM_INT3 = 0x10000007,
		ITEM_UINT3 = 0x10000008,
		ITEM_INT = 0x10000009,
		ITEM_UINT = 0x1000000A,
		ITEM_FLOAT = 0x1000000B,
		ITEM_STRING = 0x1000000C,   
		ITEM_WSTRING = 0x1000000D    
	};

	struct Item
	{
		void*          m_pValue;     
		eItemType       m_Type;
	};

	uint32_t m_iCurrentPos;
	uint16_t m_Size;
	uint16_t m_ExtraItemsCapacity;
	Item     m_ContainedItems[2];  
	Item*    m_pExtraItems;        
#endif
};

class CMwEngineInfo
{
public:
	virtual ~CMwEngineInfo() {}

	uint32_t EngineID;
	char* EngineName;
#ifdef GAMEBOX
	uint32_t Padding;
#endif
	uint32_t ClassesAmount; // ?? Classes already has this info
#ifdef TMCN
	CFastArray<CMwClassInfo*> Classes;
#else
	CMwClassInfo** Classes;
#endif
};

class CMwEngineManager
{
public:
	virtual ~CMwEngineManager() {}

#ifdef GAMEBOX
	uint32_t Padding;
	uint32_t Padding2;
#endif

	CFastArray<CMwEngineInfo*> Engines;
};

#ifdef TMCN
template <typename WantedType>
consteval
CMwStack::eItemType TypeToStackItemType()
{
	if constexpr (std::is_same_v<WantedType, bool>)
	{
		return CMwStack::ITEM_BOOL;
	}
	else if constexpr (std::is_same_v<WantedType, CMwNod*>)
	{
		return CMwStack::ITEM_OBJECT;
	}
	else if constexpr (std::is_enum_v<WantedType>)
	{
		return CMwStack::ITEM_ENUM;
	}
	else if constexpr (std::is_same_v<WantedType, int32_t>)
	{
		return CMwStack::ITEM_INT;
	}
	else if constexpr (std::is_same_v<WantedType, uint32_t>)
	{
		return CMwStack::ITEM_UINT;
	}
	else if constexpr (std::is_same_v<WantedType, float>)
	{
		return CMwStack::ITEM_FLOAT;
	}
	else
	{
		static_assert(false and "Must provide a valid and implemented type for TypeToStackItemType");
	}
}
#endif

#pragma warning(pop) // C4100