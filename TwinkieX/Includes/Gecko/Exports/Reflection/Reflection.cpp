
#include "pch.h"
#include "Reflection.h"
#include <Gecko/Exports/GeckoAutoExport.h>
#include <Gecko/Exports/Trackmania/Trackmania.h>
#include <Twinkie/Twinkie.h>

namespace Gecko::Exports::Reflection
{
	using Gecko::Exports::Trackmania::ClassIDToInfo;
	using Gecko::Exports::Trackmania::ClassNameToInfo;

	static std::vector<GeckoExport> s_Exports;
#define REFLECTION_FN(Decl_, Fn_)                     GECKO_EXPORT_FN(s_Exports, "Reflection", Decl_, Fn_)
#define REFLECTION_METHOD(Class_, Decl_, Ptr_, Conv_) GECKO_EXPORT_METHOD(s_Exports, "Reflection", Class_, Decl_, Ptr_, Conv_)
#define REFLECTION_PROP(Class_, Decl_, Offset_)       GECKO_EXPORT_PROP(s_Exports, "Reflection", Class_, Decl_, Offset_)

	// MwMemberInfo
	/*
	!CompatMeta
	for     Reflection
	with    uint ID
	as      const uint ID
	being   prop
	of      MwMemberInfo
	declare full
	since   v0.1.0
	!CompatMeta
	*/
	REFLECTION_PROP("MwMemberInfo", "const uint ID", asOFFSET(CMwMemberInfo, MemberID));
	/*
	!CompatMeta
	for     Reflection
	with    uint16 Offset
	as      const uint16 Offset
	being   prop
	of      MwMemberInfo
	declare full
	since   v0.1.0
	!CompatMeta
	*/
	REFLECTION_PROP("MwMemberInfo", "const uint16 Offset", asOFFSET(CMwMemberInfo, MemberOffset));

	// MwClassInfo

	/*
	!CompatMeta
	for     Reflection
	with    uint ID
	as      uint ID
	being   prop
	of      MwClassInfo
	declare full
	since   v0.1.0
	!CompatMeta
	*/
	REFLECTION_PROP("MwClassInfo", "const uint ID", asOFFSET(CMwClassInfo, ClassID));

	/*
	!CompatMeta
	for     Reflection
	with    const Reflection::MwClassInfo@ BaseType
	as      const Reflection::MwClassInfo@ BaseType
	being   prop
	of      MwClassInfo
	declare full
	since   v0.1.0
	!CompatMeta
	*/
	REFLECTION_PROP("MwClassInfo", "const MwClassInfo@ BaseType", asOFFSET(CMwClassInfo, ParentClassInfo));

	REFLECTION_METHOD("MwClassInfo", "string get_Name() property", asMETHOD(CMwClassInfo, GetName), asCALL_THISCALL);
#ifdef MANIAPLANET
	/*
	!CompatMeta
	for     Reflection
	with    uint Size
	as      uint Size
	being   prop
	of      MwClassInfo
	declare full
	since   v0.1.0
	games   TMCN
	!CompatMeta
	*/
	REFLECTION_PROP("MwClassInfo", "const uint Size", asOFFSET(CMwClassInfo, Size));
#endif

	/*
	!CompatMeta
	for     Reflection
	with    const Reflection::MwMemberInfo@ GetMember(const string&in name) const
	as      const Reflection::MwMemberInfo@ GetMember(const string&in name)
	being   method
	of      MwClassInfo
	declare full
	since   v0.1.0
	!CompatMeta
	*/
	void Reflection_MwClassInfo_GetMember(asIScriptGeneric* Script)
	{
		CMwClassInfo* ClassInfo = (CMwClassInfo*)Script->GetObject();
		const std::string& Name = *(const std::string*)Script->GetArgObject(0);

		for (auto& Member : *ClassInfo)
		{
			if (Member->GetName() == Name)
			{
				Script->SetReturnAddress(Member);
				return;
			}
		}

		Script->SetReturnAddress(nullptr);
	}
	REFLECTION_METHOD("MwClassInfo", "MwMemberInfo@ GetMember(const string&in name)", asFUNCTION(Reflection_MwClassInfo_GetMember), asCALL_GENERIC);

	/*
	!CompatMeta
	for     Reflection
	with    const Reflection::MwMemberInfo@[]@ get_Members() const
	as      const Reflection::MwMemberInfo@[]@ get_Members()
	being   method
	of      MwClassInfo
	declare full
	since   v0.1.0
	!CompatMeta
	*/
	void Reflection_MwClassInfo_get_Members(asIScriptGeneric* Script)
	{
		asIScriptEngine* Engine = Script->GetEngine();
		CMwClassInfo* ClassInfo = (CMwClassInfo*)Script->GetObject();

		if (!ClassInfo->Members)
		{
			Script->SetReturnObject(nullptr);
			return;
		}

		asIScriptFunction* Function = Script->GetFunction();

		asITypeInfo* ArrayType = Engine->GetTypeInfoById(Function->GetReturnTypeId());

		CScriptArray* Array = CScriptArray::Create(ArrayType, ClassInfo->MembersAmount);

		uint32_t Idx = 0;
		for (auto& Member : *ClassInfo)
		{
			Array->SetValue(Idx, &Member);
		}

		Script->SetReturnObject(Array);
	}
	REFLECTION_METHOD("MwClassInfo", "const MwMemberInfo@[]@ get_Members() property", asFUNCTION(Reflection_MwClassInfo_get_Members), asCALL_GENERIC);

#ifdef MANIAPLANET
	/*
	!CompatMeta
	for     Reflection
	with    const string get_FileExtension() const
	as      const string get_FileExtension() const
	being   method
	of      MwClassInfo
	declare full
	since   v0.1.0
	games   TMCN
	!CompatMeta
	*/
	std::string Reflection_MwClassInfo_get_FileExtension(CMwClassInfo* Class)
	{
		return Class->FileExtName ? "" : Class->FileExtName;
	}
	REFLECTION_METHOD("MwClassInfo", "string get_FileExtension() property", asFUNCTION(Reflection_MwClassInfo_get_FileExtension), asCALL_CDECL_OBJFIRST);

	/*
	!CompatMeta
	for     Reflection
	with    const string get_Description() const
	as      const string get_Description()
	being   method
	of      MwClassInfo
	declare full
	since   v0.1.0
	games   TMCN
	!CompatMeta
	*/
	std::string Reflection_MwClassInfo_get_Description(CMwClassInfo* Class)
	{
		if (Class->Description)
		{
			if (*Class->Description)
			{
				return *Class->Description;
			}
			return "";
		}
		else
		{
			return "";
		}
	}
	REFLECTION_METHOD("MwClassInfo", "string get_Description() property", asFUNCTION(Reflection_MwClassInfo_get_Description), asCALL_CDECL_OBJFIRST);
#endif

	/*
	!CompatMeta
	for     Reflection
	with    const MwClassInfo@ GetType(uint id)
	as      const MwClassInfo@ GetType(uint id)
	being   function
	declare full
	since   v0.1.0
	!CompatMeta
	*/
	CMwClassInfo* Reflection_GetType_ById(uint32_t id)
	{
		return ClassIDToInfo[id];
	}
	REFLECTION_FN("MwClassInfo@ GetType(uint id)", Reflection_GetType_ById);

	/*
	!CompatMeta
	for     Reflection
	with    const MwClassInfo@ GetType(const string&in name)
	as      const MwClassInfo@ GetType(const string&in name)
	being   function
	declare full
	since   v0.1.0
	!CompatMeta
	*/
	CMwClassInfo* Reflection_GetType_ByName(const std::string& name)
	{
		return ClassNameToInfo[name];
	}
	REFLECTION_FN("MwClassInfo@ GetType(const string&in name)", Reflection_GetType_ByName);

	/*
	!CompatMeta
	for     Reflection
	with    const MwClassInfo@ TypeOf(CMwNod@ nod)
	as      const MwClassInfo@ TypeOf(CMwNod@ nod)
	being   function
	declare full
	since   v0.1.0
	!CompatMeta
	*/
	CMwClassInfo* Reflection_TypeOf(CMwNod* nod)
	{
		return nod->MwGetClassInfo();
	}
	REFLECTION_FN("MwClassInfo@ TypeOf(CMwNod@ nod)", Reflection_TypeOf);

	/*
	!CompatMeta
	for     Reflection
	with    int GetRefCount(CMwNod@ nod)
	as      int GetRefCount(CMwNod@ nod)
	being   function
	declare full
	since   v0.1.0
	!CompatMeta
	*/
	int Reflection_GetRefCount(CMwNod* nod)
	{
		return nod->ReferenceCount;
	}
	REFLECTION_FN("int GetRefCount(CMwNod@ nod)", Reflection_GetRefCount);

	void Registrar(asIScriptEngine* Engine)
	{
		// Types have to exist before anything in s_Exports can reference them.
		Engine->SetDefaultNamespace("Reflection");
		Engine->RegisterObjectType("MwMemberInfo", 0, asOBJ_REF | asOBJ_NOCOUNT);
		Engine->RegisterObjectType("MwClassInfo", 0, asOBJ_REF | asOBJ_NOCOUNT);
		Engine->SetDefaultNamespace("");

		RegisterAll(Engine, s_Exports);
	}
}
