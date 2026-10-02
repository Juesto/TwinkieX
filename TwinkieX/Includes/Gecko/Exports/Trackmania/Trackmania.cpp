#include "pch.h"
#include "Trackmania.h"
#include <Twinkie/Twinkie.h>

#undef GetObject
#undef RegisterClass

namespace Gecko::Exports::Trackmania
{
	std::unordered_map<uint32_t, CMwClassInfo*> ClassIDToInfo;
	std::unordered_map<std::string, CMwClassInfo*> ClassNameToInfo;

	namespace BaseTypes
	{
		static void __cdecl CMwNod_AddRef(CMwNod* Nod)
		{
			Nod->ReferenceCount++;
		}

		static void __cdecl CMwNod_RemoveRef(CMwNod* Nod)
		{
			Nod->ReferenceCount--;
		}

		static uint32_t __cdecl CFastArray_Length(CFastArrayGen* Array)
		{
			return Array->Size;
		}

		static void __cdecl CFastArray_opIndex(asIScriptGeneric* Script)
		{
			auto Engine = Script->GetEngine();

			auto ArrayType = Engine->GetTypeInfoById(Script->GetObjectTypeId());
			auto SubTypeId = ArrayType->GetSubTypeId();

			uint32_t ElemSize = 0xFFFFFFFF;
			if ((SubTypeId & asTYPEID_MASK_OBJECT) == 0)
			{
				ElemSize = Engine->GetSizeOfPrimitiveType(SubTypeId);
			}
			else if (SubTypeId & asTYPEID_OBJHANDLE)
			{
				ElemSize = sizeof(void*);
			}
			else
			{
				auto SubType = Engine->GetTypeInfoById(SubTypeId);
				if (SubType->GetFlags() & asOBJ_VALUE)
					ElemSize = SubType->GetSize();
				else
					ElemSize = sizeof(void*);
			}

			uint32_t Idx = Script->GetArgDWord(0);
			CFastArrayGen* Array = (CFastArrayGen*)Script->GetObject();

			if (Idx >= Array->Size) (void)0; // TODO: Raise an exception when OOB

			Script->SetReturnAddress(Array->Get(Idx, ElemSize));
		}

		// Registers all base types used by the game.
		static void RegistrarBaseTypes(asIScriptEngine* Engine)
		{
			Engine->RegisterObjectType("MwFastArray<T>", 0, asOBJ_REF | asOBJ_NOCOUNT | asOBJ_TEMPLATE);
			Engine->RegisterObjectMethod("MwFastArray<T>", "const uint get_Length() property", asFUNCTION(CFastArray_Length), asCALL_CDECL_OBJLAST);
			Engine->RegisterObjectMethod("MwFastArray<T>", "T& opIndex(uint index)", asFUNCTION(CFastArray_opIndex), asCALL_GENERIC);
			Engine->RegisterObjectMethod("MwFastArray<T>", "const T& opIndex(uint index) const", asFUNCTION(CFastArray_opIndex), asCALL_GENERIC);
		}
	}

	// CGameApp@ GetApp()
	static CMwNod* __stdcall GetApp()
	{
		CMwNod* App = ReadAddr(CMwNod*, (uintptr_t)GetModuleHandleA(NULL) + O_APP);
		App->ReferenceCount++;
		return App;
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
				BaseTypes::CMwNod_AddRef(FromNod);
				Script->SetReturnAddress(FromNod);
				return;
			}
			From = From->ParentClassInfo;
		}
		Script->SetReturnAddress(nullptr);
	}

	void VirtualGetAs(asIScriptGeneric* Script)
	{
		CMwNod* Nod = (CMwNod*)Script->GetObject();
		CMwMemberInfo* Member = (CMwMemberInfo*)Script->GetAuxiliary();

		// TODO: Add other types.
		if (Member->MemberType == CMwMemberInfo::CLASS)
		{
			Script->SetReturnAddress(gTwinkie.TrackmaniaMgr.ParamGet<CMwNod*>(Nod, Member));
		}
	}

	void CasterNonPersistent(asIScriptGeneric* Script)
	{
		Script->SetReturnAddress(nullptr);
	}

	static inline void RegisterClass(asIScriptEngine* Engine, CMwClassInfo* Class)
	{
		if (Engine->GetTypeInfoByName(Class->GetName().c_str())) return;
		ClassIDToInfo[Class->ClassID] = Class;
		ClassNameToInfo[Class->GetName()] = Class;
		Engine->RegisterObjectType(Class->GetName().c_str(), 0, asOBJ_REF);

		if (Class->CtorFn) Engine->RegisterObjectBehaviour(Class->GetName().c_str(), asBEHAVE_FACTORY, (Class->GetName() + std::string("@ f()")).c_str(), (uintptr_t)Class->CtorFn, asCALL_CDECL);
		Engine->RegisterObjectBehaviour(Class->GetName().c_str(), asBEHAVE_ADDREF, "void f()", asFUNCTION(BaseTypes::CMwNod_AddRef), asCALL_CDECL_OBJFIRST);
		Engine->RegisterObjectBehaviour(Class->GetName().c_str(), asBEHAVE_RELEASE, "void f()", asFUNCTION(BaseTypes::CMwNod_RemoveRef), asCALL_CDECL_OBJFIRST);
	}

	static void RegisterMemberVirtual(asIScriptEngine* Engine, CMwClassInfo* Class, CMwMemberInfo* Member)
	{
		using enum CMwMemberInfo::eType;

		// TODO: Add more types.
		switch (Member->MemberType)
		{
		case CLASS:
			Engine->RegisterObjectMethod(
				Class->GetName().c_str(),
				std::format(
					"{}@ get_{}() property",
					((CMwMemberInfoClass*)Member)->ClassInfo->GetName(), Member->GetName()
				).c_str(),
				asFUNCTION(VirtualGetAs),
				asCALL_GENERIC,
				Member
			);
			break;
		default:
			break;
		}
	}

	static void RegisterMemberNormal(asIScriptEngine* Engine, CMwClassInfo* Class, CMwMemberInfo* Member)
	{
		using enum CMwMemberInfo::eType;

		// TODO: Add more types.
		switch (Member->MemberType)
		{
		case CLASS:
		{
			Engine->RegisterObjectProperty(
				Class->GetName().c_str(),
				std::format(
					"{}@ {}",
					((CMwMemberInfoClass*)Member)->ClassInfo->GetName(), Member->GetName()
				).c_str(),
				Member->MemberOffset
			);
			break;
		}
		case CLASSARRAY:
		{
			Engine->RegisterObjectProperty(
				Class->GetName().c_str(),
				std::format(
					"MwFastArray<{}>@ {}",
					((CMwMemberInfoClassArray*)Member)->ArrayClassInfo->GetName(), Member->GetName()
				).c_str(),
				Member->MemberOffset
			);
			break;
		}
		case BOOL:
		case INT:
		case NATURAL:
		case REAL:
		{
			Engine->RegisterObjectProperty(
				Class->GetName().c_str(),
				std::format(
					"{} {}",
					g_MemberTypeSignatures[Member->MemberType], 
					Member->GetName()
				).c_str(),
				Member->MemberOffset
			);
			break;
		}
		default:
		{
			break;
		}
		}
	}

	// Registers all classes available from the engine manager.
	void RegistrarFromTm(asIScriptEngine* Engine)
	{
		gTwinkie.TrackmaniaMgr.GetApp()->ReferenceCount++;

		CMwEngineManager* EngineMgr = gTwinkie.TrackmaniaMgr.GetEngineManager();

		for (auto& TmEngine : EngineMgr->Engines)
		{
			if (!TmEngine) continue;

#ifdef GAMEBOX
			for (uint32_t ClassIdx = 0; ClassIdx < TmEngine->ClassesAmount; ClassIdx++)
#else
			for (auto& Class : TmEngine->Classes)
#endif
			{
#ifdef GAMEBOX
				auto Class = TmEngine->Classes[ClassIdx];
#endif
				if (!Class) continue;

				RegisterClass(Engine, Class);
			}
		}
		for (auto& TmEngine : EngineMgr->Engines)
		{
			if (!TmEngine) continue;

#ifdef GAMEBOX
			for (uint32_t ClassIdx = 0; ClassIdx < TmEngine->ClassesAmount; ClassIdx++)
#else
			for (auto& Class : TmEngine->Classes)
#endif
			{
#ifdef GAMEBOX
				auto Class = TmEngine->Classes[ClassIdx];
#endif
				if (!Class) continue;

				CMwClassInfo* Parent = Class;
				while (Parent)
				{
					for (uint32_t MemberIdx = 0; MemberIdx < Parent->MembersAmount; MemberIdx++)
					{
						CMwMemberInfo* Member = Parent->Members[MemberIdx];

						if (auto TypeInfo = Engine->GetTypeInfoByName(Class->GetName().c_str()))
						{
							bool SkipMember = false;
							for (uint32_t AsMemberIdx = 0; AsMemberIdx < TypeInfo->GetPropertyCount(); AsMemberIdx++)
							{
								const char* AsMemberName = nullptr;
								TypeInfo->GetProperty(AsMemberIdx, &AsMemberName);

								if (strcmp(AsMemberName, Member->GetName().c_str()) == 0)
								{
									SkipMember = true;
									break;
								}
							}
							if (SkipMember) continue;
						}

						// Non-virtual
						// TODO: Use Member->MemberFlags0/1 to know if to use VirtualGet/Set.
						if (Member->MemberOffset <= 32767)
							RegisterMemberNormal(Engine, Class, Member);
						else
							RegisterMemberVirtual(Engine, Class, Member);
					}
					Parent = Parent->ParentClassInfo;
				}
			}
		}
		for (auto& TmEngine : EngineMgr->Engines)
		{
			if (!TmEngine) continue;

#ifdef GAMEBOX
			for (uint32_t ClassIdx = 0; ClassIdx < TmEngine->ClassesAmount; ClassIdx++)
#else
			for (auto& Class : TmEngine->Classes)
#endif
			{
#ifdef GAMEBOX
				auto Class = TmEngine->Classes[ClassIdx];
#endif
				if (!Class) continue;

				CMwClassInfo* Parent = Class->ParentClassInfo;
				bool HasMwNodParent = false;
				Parent = Class->ParentClassInfo;
				while (Parent)
				{
					Engine->RegisterObjectMethod(
						Parent->GetName().c_str(),
						std::format(
							"{}@ opCast()",
							Class->GetName()
						).c_str(),
						asFUNCTION(Caster),
						asCALL_GENERIC,
						Class
					);

					Engine->RegisterObjectMethod(
						Class->GetName().c_str(),
						std::format(
							"{}@ opCast()",
							Parent->GetName()
						).c_str(),
						asFUNCTION(Caster),
						asCALL_GENERIC,
						Parent
					);

					if (Parent->GetName() == "CMwNod")
						HasMwNodParent = true;

					Parent = Parent->ParentClassInfo;
				}
				Parent = Class->ParentClassInfo;
				while (Parent && HasMwNodParent)
				{
					if (Engine->GetTypeInfoByDecl(Class->GetName().c_str())->GetMethodByDecl("CMwNod@ opImplCast()"))
					{
						Parent = Parent->ParentClassInfo;
						continue;
					}

					Engine->RegisterObjectMethod(
						Class->GetName().c_str(),
						"CMwNod@ opImplCast()",
						asFUNCTION(Caster),
						asCALL_GENERIC,
						Parent
					);

					Parent = Parent->ParentClassInfo;
				}
			}
#ifdef _DEBUG
			std::cout << "Registered engine " << TmEngine->EngineName << " (ID 0x" << std::hex << TmEngine->EngineID << std::dec << ")\n";
#endif
		}
	}

	void Registrar(asIScriptEngine* Engine)
	{
		BaseTypes::RegistrarBaseTypes(Engine);
		RegistrarFromTm(Engine);

		Engine->RegisterGlobalFunction("CGameApp@ GetApp()", asFUNCTION(GetApp), asCALL_STDCALL);
	}

	void Cleanup()
	{
		GetApp()->ReferenceCount--;
	}
}