#include "pch.h"
#include "Trackmania.h"
#include <Twinkie/Twinkie.h>

#undef GetObject
#undef RegisterClass

namespace Gecko::Exports::Trackmania
{
	std::unordered_map<uint32_t, CMwClassInfo*> ClassIDToInfo;

	static void __cdecl CMwNod_AddRef(CMwNod* Nod)
	{
		Nod->ReferenceCount++;
	}

	static void __cdecl CMwNod_RemoveRef(CMwNod* Nod)
	{
		Nod->ReferenceCount--;
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
				CMwNod_AddRef(FromNod);
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
		std::cout << "Registered " << Class->GetName() << "\n";
		Engine->RegisterObjectType(Class->GetName().c_str(), 0, asOBJ_REF);

		if (Class->CtorFn) Engine->RegisterObjectBehaviour(Class->GetName().c_str(), asBEHAVE_FACTORY, (Class->GetName() + std::string("@ f()")).c_str(), (uintptr_t)Class->CtorFn, asCALL_CDECL);
		Engine->RegisterObjectBehaviour(Class->GetName().c_str(), asBEHAVE_ADDREF, "void f()", asFUNCTION(CMwNod_AddRef), asCALL_CDECL_OBJFIRST);
		Engine->RegisterObjectBehaviour(Class->GetName().c_str(), asBEHAVE_RELEASE, "void f()", asFUNCTION(CMwNod_RemoveRef), asCALL_CDECL_OBJFIRST);
	}

	void Registrar(asIScriptEngine* Engine)
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

						// TODO: Add other types.
						if (Member->MemberOffset <= 32767 && Member->MemberType == CMwMemberInfo::CLASS)
							Engine->RegisterObjectProperty(
								Class->GetName().c_str(),
								std::format(
									"{}@ {}",
									((CMwMemberInfoClass*)Member)->ClassInfo->GetName(), Member->GetName()
								).c_str(),
								Member->MemberOffset
							);
							
						else if (Member->MemberOffset > 32767 && Member->MemberType == CMwMemberInfo::CLASS)
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

						std::cout << "Registered " << Member->GetName() << " for " << Class->GetName() << "\n";
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

					Parent = Parent->ParentClassInfo;
				}
			}
		}

		Engine->RegisterGlobalFunction("CGameApp@ GetApp()", asFUNCTION(GetApp), asCALL_STDCALL);
	}

	void Cleanup()
	{
		GetApp()->ReferenceCount--;
	}
}