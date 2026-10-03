#include "pch.h"
#include "Trackmania.h"
#include <Twinkie/Twinkie.h>
#include <unordered_set>

#undef GetObject
#undef RegisterClass

namespace Gecko::Exports::Trackmania
{
	std::unordered_map<uint32_t, CMwClassInfo*> ClassIDToInfo;
	std::unordered_map<std::string, CMwClassInfo*> ClassNameToInfo;

	namespace BaseTypes
	{
		//static void __cdecl CMwNod_AddRef(CMwNod* Nod)
		//{
		//	Nod->ReferenceCount++;
		//}

		//static void __cdecl CMwNod_RemoveRef(CMwNod* Nod)
		//{
		//	Nod->ReferenceCount--;
		//}

		static uint32_t __cdecl CFastArray_Length(CFastArrayGen* Array)
		{
			return Array->Size;
		}

		// Size in bytes of one element of a MwFastArray<T> instance
		static uint32_t CFastArray_ElemSize(asIScriptEngine* Engine, asITypeInfo* ArrayType)
		{
			int SubTypeId = ArrayType->GetSubTypeId();

			if ((SubTypeId & asTYPEID_MASK_OBJECT) == 0)
				return Engine->GetSizeOfPrimitiveType(SubTypeId);

			if (SubTypeId & asTYPEID_OBJHANDLE)
				return sizeof(void*);

			auto SubType = Engine->GetTypeInfoById(SubTypeId);
			if (SubType->GetFlags() & asOBJ_VALUE)
				return SubType->GetSize();

			return sizeof(void*);
		}

		static void __cdecl CFastArray_opIndex(asIScriptGeneric* Script)
		{
			auto Engine = Script->GetEngine();
			uint32_t ElemSize = CFastArray_ElemSize(Engine, Engine->GetTypeInfoById(Script->GetObjectTypeId()));

			uint32_t Idx = Script->GetArgDWord(0);
			CFastArrayGen* Array = (CFastArrayGen*)Script->GetObject();

			if (Idx >= Array->Size) (void)0; // TODO: Raise an exception when OOB

			Script->SetReturnAddress(Array->Get(Idx, ElemSize));
		}

		static bool __cdecl CFastArray_TemplateCallback(asITypeInfo* ArrayType, bool& DontGarbageCollect)
		{
			DontGarbageCollect = true;

			int SubTypeId = ArrayType->GetSubTypeId();
			if (SubTypeId == asTYPEID_VOID) return false;

			if ((SubTypeId & asTYPEID_MASK_OBJECT) && !(SubTypeId & asTYPEID_OBJHANDLE))
			{
				auto SubType = ArrayType->GetEngine()->GetTypeInfoById(SubTypeId);
				if ((SubType->GetFlags() & asOBJ_VALUE) && !(SubType->GetFlags() & asOBJ_POD))
				{
					// TODO: Raise an error here
					return false;
				}
			}

			return true;
		}

		// All arrays made by a script
		static std::unordered_set<CFastArrayGen*> OwnedArrays;

		static void CFastArray_FreeBuffer(asIScriptEngine* Engine, asITypeInfo* ArrayType, CFastArrayGen* Array)
		{
			if (not Array->Ptr) return;

			int SubTypeId = ArrayType->GetSubTypeId();
			if (SubTypeId & asTYPEID_OBJHANDLE)
			{
				auto SubType = Engine->GetTypeInfoById(SubTypeId);
				auto Elems = (void**)Array->Ptr;
				for (uint32_t i = 0; i < Array->Size; i++)
					if (Elems[i]) Engine->ReleaseScriptObject(Elems[i], SubType);
			}

			delete[] (uint8_t*)Array->Ptr;
			Array->Ptr = nullptr;
			Array->Size = 0;
		}

		static void CFastArray_Construct(asITypeInfo* ArrayType, uint32_t Size, void* Memory)
		{
			CFastArrayGen* NewArray = new (Memory) CFastArrayGen();
			OwnedArrays.insert(NewArray);

#ifdef GAMEBOX
			// Also copy the vftable because yes, arrays have vftables now
			// Evil hack incoming
			static uintptr_t CachedVf = 0;
			if (!CachedVf)
			{
				auto& Twinkie = gTwinkie.TrackmaniaMgr;
				auto Resources = Twinkie.ParamGet<CMwNod*>(Twinkie.GetApp(), "Resources");
				auto AudioSources = Resources ? Twinkie.ParamGet<CFastArrayGen>(Resources, "AudioSources") : nullptr;
				if (AudioSources) CachedVf = AudioSources->vf;
			}

			// TODO: Something better ^^
			NewArray->vf = CachedVf;
#endif
			if (Size == 0) return;

			uint32_t ElemSize = CFastArray_ElemSize(ArrayType->GetEngine(), ArrayType);

			NewArray->Ptr = new uint8_t[(size_t)Size * ElemSize]();
			NewArray->Size = Size;
		}

		static void CFastArray_ctor(asIScriptGeneric* Script)
		{
			asITypeInfo* ArrayType = *(asITypeInfo**)Script->GetAddressOfArg(0);
			CFastArray_Construct(ArrayType, 0, Script->GetObject());
		}

		static void CFastArray_ctorSize(asIScriptGeneric* Script)
		{
			asITypeInfo* ArrayType = *(asITypeInfo**)Script->GetAddressOfArg(0);
			CFastArray_Construct(ArrayType, Script->GetArgDWord(1), Script->GetObject());
		}

		static void CFastArray_dtor(asIScriptGeneric* Script)
		{
			auto Engine = Script->GetEngine();
			auto Array = (CFastArrayGen*)Script->GetObject();

			if (!OwnedArrays.erase(Array)) return;

			CFastArray_FreeBuffer(Engine, Engine->GetTypeInfoById(Script->GetObjectTypeId()), Array);
		}


		static void CFastArray_opAssign(asIScriptGeneric* Script)
		{
			auto Engine = Script->GetEngine();
			auto Dst = (CFastArrayGen*)Script->GetObject();
			auto Src = (CFastArrayGen*)Script->GetArgObject(0);

			Script->SetReturnAddress(Dst);

			if (Dst == Src) return;

			if (!OwnedArrays.contains(Dst))
			{
				if (auto Ctx = asGetActiveContext()) (void)0; // TODO: Raise an error, cannot assign to nod's array
				return;
			}

			auto ArrayType = Engine->GetTypeInfoById(Script->GetObjectTypeId());
			int SubTypeId = ArrayType->GetSubTypeId();
			uint32_t ElemSize = CFastArray_ElemSize(Engine, ArrayType);

			uint8_t* NewPtr = nullptr;
			uint32_t NewSize = (Src->Ptr ? Src->Size : 0);
			if (NewSize)
			{
				NewPtr = new uint8_t[(size_t)NewSize * ElemSize];
				memcpy(NewPtr, Src->Ptr, (size_t)NewSize * ElemSize);

				if (SubTypeId & asTYPEID_OBJHANDLE)
				{
					auto SubType = Engine->GetTypeInfoById(SubTypeId);
					auto Elems = (void**)NewPtr;
					for (uint32_t i = 0; i < NewSize; i++)
						if (Elems[i]) Engine->AddRefScriptObject(Elems[i], SubType);
				}
			}

			CFastArray_FreeBuffer(Engine, ArrayType, Dst);

			Dst->Ptr = NewPtr;
			Dst->Size = NewSize;
		}

		// Registers all base types used by the game.
		static void RegistrarBaseTypes(asIScriptEngine* Engine)
		{
			Engine->RegisterObjectType("MwFastArray<T>", sizeof(CFastArrayGen), asOBJ_VALUE | asOBJ_TEMPLATE | asGetTypeTraits<CFastArrayGen>());
			Engine->RegisterObjectBehaviour("MwFastArray<T>", asBEHAVE_TEMPLATE_CALLBACK, "bool f(int&in, bool&out)", asFUNCTION(CFastArray_TemplateCallback), asCALL_CDECL);
			Engine->RegisterObjectBehaviour("MwFastArray<T>", asBEHAVE_CONSTRUCT, "void f(int&in)", asFUNCTION(CFastArray_ctor), asCALL_GENERIC);
			Engine->RegisterObjectBehaviour("MwFastArray<T>", asBEHAVE_CONSTRUCT, "void f(int&in, uint)", asFUNCTION(CFastArray_ctorSize), asCALL_GENERIC);
			Engine->RegisterObjectBehaviour("MwFastArray<T>", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(CFastArray_dtor), asCALL_GENERIC);
			Engine->RegisterObjectMethod("MwFastArray<T>", "MwFastArray<T>& opAssign(const MwFastArray<T>&in)", asFUNCTION(CFastArray_opAssign), asCALL_GENERIC);
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
		Engine->RegisterObjectType(Class->GetName().c_str(), 0, asOBJ_REF | asOBJ_NOCOUNT);

		if (Class->CtorFn) Engine->RegisterObjectBehaviour(Class->GetName().c_str(), asBEHAVE_FACTORY, (Class->GetName() + std::string("@ f()")).c_str(), (uintptr_t)Class->CtorFn, asCALL_CDECL);
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
					"MwFastArray<{}@> {}",
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