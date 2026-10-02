#include "pch.h"
#include "Reflection.h"
#include <Gecko/Exports/Trackmania/Trackmania.h>
#include <Twinkie/Twinkie.h>

namespace Gecko::Exports::Reflection
{
	using Gecko::Exports::Trackmania::ClassIDToInfo;
	using Gecko::Exports::Trackmania::ClassNameToInfo;
	
	// const Reflection::MwMemberInfo@ GetMember(const string&in name)
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

	// const Reflection::MwMemberInfo@[]@ Reflection::MwClassInfo::get_Members()
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

		assert(ArrayType);
		assert(std::string(ArrayType->GetName()) == "array");

		CScriptArray* Array = CScriptArray::Create(ArrayType, ClassInfo->MembersAmount);

		uint32_t Idx = 0;
		for (auto& Member : *ClassInfo)
		{
			Array->SetValue(Idx, &Member);
		}

		Script->SetReturnObject(Array);
	}

#ifdef MANIAPLANET
	std::string Reflection_MwClassInfo_get_FileExtension(CMwClassInfo* Class)
	{
		return Class->FileExtName ? "" : Class->FileExtName;
	}

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
#endif

	// Reflection::MwClassInfo@ Reflection::GetType(uint id)
	CMwClassInfo* Reflection_GetType_ById(uint32_t id)
	{
		return ClassIDToInfo[id];
	}

	// Reflection::MwClassInfo@ Reflection::GetType(const string&in name)
	CMwClassInfo* Reflection_GetType_ByName(const std::string& name)
	{
		return ClassNameToInfo[name];
	}

	// Reflection::MwClassInfo@ Reflection::TypeOf(CMwNod@ nod)
	CMwClassInfo* Reflection_TypeOf(CMwNod* nod)
	{
		return nod->MwGetClassInfo();
	}

	// int Reflection::GetRefCount(CMwNod@ nod)
	int Reflection_GetRefCount(CMwNod* nod)
	{
		return nod->ReferenceCount;
	}

	void Registrar(asIScriptEngine* Engine)
	{
		Engine->SetDefaultNamespace("Reflection");

		Engine->RegisterObjectType("MwMemberInfo", 0, asOBJ_REF | asOBJ_NOCOUNT);
		Engine->RegisterObjectProperty("MwMemberInfo", "const uint ID", asOFFSET(CMwMemberInfo, MemberID));
		Engine->RegisterObjectProperty("MwMemberInfo", "const uint16 Offset", asOFFSET(CMwMemberInfo, MemberOffset));
		Engine->RegisterObjectMethod("MwMemberInfo", "string get_NameDescriptive() property", asMETHOD(CMwMemberInfo, GetName), asCALL_THISCALL);

		Engine->RegisterObjectType("MwClassInfo", 0, asOBJ_REF | asOBJ_NOCOUNT);
		Engine->RegisterObjectProperty("MwClassInfo", "const uint ID", asOFFSET(CMwClassInfo, ClassID));
		Engine->RegisterObjectProperty("MwClassInfo", "const MwClassInfo@ BaseType", asOFFSET(CMwClassInfo, ParentClassInfo));
		Engine->RegisterObjectMethod("MwClassInfo", "MwMemberInfo@ GetMember(const string&in name)", asFUNCTION(Reflection_MwClassInfo_GetMember), asCALL_GENERIC);
		Engine->RegisterObjectMethod("MwClassInfo", "const MwMemberInfo@[]@ get_Members() property", asFUNCTION(Reflection_MwClassInfo_get_Members), asCALL_GENERIC);
		Engine->RegisterObjectMethod("MwClassInfo", "string get_Name() property", asMETHOD(CMwClassInfo, GetName), asCALL_THISCALL);
#ifdef MANIAPLANET
		// TODO: Figure out where UserName and ChildClasses go in this
		Engine->RegisterObjectProperty("MwClassInfo", "const uint Size", asOFFSET(CMwClassInfo, Size));
		Engine->RegisterObjectMethod("MwClassInfo", "string get_FileExtension() property", asFUNCTION(Reflection_MwClassInfo_get_FileExtension), asCALL_CDECL_OBJFIRST);
		Engine->RegisterObjectMethod("MwClassInfo", "string get_Description() property", asFUNCTION(Reflection_MwClassInfo_get_Description), asCALL_CDECL_OBJFIRST);
#endif

		Engine->RegisterGlobalFunction("MwClassInfo@ GetType(uint id)", asFUNCTION(Reflection_GetType_ById), asCALL_CDECL);
		Engine->RegisterGlobalFunction("MwClassInfo@ GetType(const string&in name)", asFUNCTION(Reflection_GetType_ByName), asCALL_CDECL);
		Engine->RegisterGlobalFunction("MwClassInfo@ TypeOf(CMwNod@ nod)", asFUNCTION(Reflection_TypeOf), asCALL_CDECL);
		Engine->RegisterGlobalFunction("int GetRefCount(CMwNod@ nod)", asFUNCTION(Reflection_GetRefCount), asCALL_CDECL);

		Engine->SetDefaultNamespace("");
	}
}