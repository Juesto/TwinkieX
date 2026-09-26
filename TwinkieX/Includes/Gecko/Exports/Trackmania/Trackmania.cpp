#include "pch.h"
#include "Trackmania.h"

#undef GetObject

namespace Gecko::Exports::Trackmania
{
	static void CMwNod_AddRef(CMwNod* Nod)
	{
		Nod->ReferenceCount++;
	}

	static void CMwNod_RemoveRef(CMwNod* Nod)
	{
		Nod->ReferenceCount--;
		if (!Nod->ReferenceCount) Nod->~CMwNod();
	}

	void Caster(asIScriptGeneric* Script)
	{
		CMwNod* FromNod = (CMwNod*)Script->GetObject();

		CMwClassInfo* To = (CMwClassInfo*)Script->GetAuxiliary();
		CMwClassInfo* From = FromNod->MwGetClassInfo();

		while (From)
		{
			if (From->ClassID == To->ClassID)
			{
				CMwNod_AddRef(FromNod);
				Script->SetReturnAddress(FromNod);
				return;
			}
			From = From->ParentClassInfo;
		}
		Script->SetReturnAddress(nullptr);
	}

	void CasterNonPersistent(asIScriptGeneric* Script)
	{
		Script->SetReturnAddress(nullptr);
	}

	static inline void RegisterClass(asIScriptEngine* Engine, CMwClassInfo* Class)
	{
		Engine->RegisterObjectType(Class->ClassName, 0, asOBJ_REF);

		if (Class->CtorFn) Engine->RegisterObjectBehaviour(Class->ClassName, asBEHAVE_FACTORY, (Class->ClassName + std::string("@ f()")).c_str(), (uintptr_t)Class->CtorFn, asCALL_CDECL);
		Engine->RegisterObjectBehaviour(Class->ClassName, asBEHAVE_ADDREF, "void f()", asFUNCTION(CMwNod_AddRef), asCALL_CDECL_OBJFIRST);
		Engine->RegisterObjectBehaviour(Class->ClassName, asBEHAVE_RELEASE, "void f()", asFUNCTION(CMwNod_RemoveRef), asCALL_CDECL_OBJFIRST);
	}

	void Registrar(asIScriptEngine* Engine)
	{
		/*for (uint32_t ClassIdx = 0; ClassIdx < ClassesAmount; ClassIdx++)
		{
			CMwClassInfo* Class = AvailableClasses[ClassIdx];

			if (!Engine->GetTypeInfoByName(Class->ClassName))
			{
				RegisterClass(Engine, Class);
			}
		}
		for (uint32_t ClassIdx = 0; ClassIdx < ClassesAmount; ClassIdx++)
		{
			CMwClassInfo* Class = AvailableClasses[ClassIdx];

			CMwClassInfo* Parent = Class;
			while (Parent)
			{
				for (uint32_t MemberIdx = 0; MemberIdx < Parent->MembersAmount; MemberIdx++)
				{
					CMwMemberInfo* Member = Parent->Members[MemberIdx];

					Engine->RegisterObjectProperty(Class->ClassName, std::format("int {}", Member->MemberName).c_str(), Member->MemberOffset);
				}
				Parent = Parent->ParentClassInfo;
			}
		}
		for (uint32_t ClassIdx = 0; ClassIdx < ClassesAmount; ClassIdx++)
		{
			CMwClassInfo* Class = AvailableClasses[ClassIdx];

			if (!Class) continue;

			CMwClassInfo* Parent = Class->ParentClassInfo;
			while (Parent)
			{
				Engine->RegisterObjectMethod(
					Parent->ClassName,
					std::format(
						"{}@ opCast()",
						Class->ClassName
					).c_str(),
					asFUNCTION(Caster),
					asCALL_GENERIC,
					Class
				);

				Engine->RegisterObjectMethod(
					Class->ClassName,
					std::format(
						"{}@ opCast()",
						Parent->ClassName
					).c_str(),
					asFUNCTION(Caster),
					asCALL_GENERIC,
					Parent
				);

				Parent = Parent->ParentClassInfo;
			}
		}*/
	}

	void Cleanup()
	{
	}
}