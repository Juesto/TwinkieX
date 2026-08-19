#include "pch.h"
#include "NodExplorerModule.h"
#include <format>

#define HasRightClickedOnItem() IsMouseClicked(ImGuiMouseButton_Right) and IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)

constexpr bool IsMemberClassAdjacent(CMwMemberInfo* Member)
{
	std::vector<CMwMemberInfo::eType> ClassTypes = {
		CMwMemberInfo::CLASS
#ifdef TMCN
		,
		CMwMemberInfo::CLASSALT,
		CMwMemberInfo::CLASSNOTPERSISTENT
#endif
	};

	return std::find(ClassTypes.begin(), ClassTypes.end(), Member->MemberType) != ClassTypes.end();
}

using namespace ImGui;

void NodExplorerModule::SetMemberInfoPopup(CMwMemberInfo* MemberInfo, const std::string NodName, CMwNod* Nod, CMwClassInfo* ClassInfo, bool IsNodImpersistent)
{
	// Fix memory leak incase of user clicking on another member while pop-up is still active
	if (ChildInfo)
	{
		delete ChildInfo;
		ChildInfo = nullptr;
	}

	if (NodInfo)
	{
		delete NodInfo;
		NodInfo = nullptr;
		NodInfoPopup = false;
	}

	this->MemberInfoPopup = true;
	ChildInfo = new PopupMemberInfo{NodName};
	ChildInfo->MemberInfo = MemberInfo;
	ChildInfo->ParentNod = Nod;
	ChildInfo->ParentClassInfo = ClassInfo;

	if (IsMemberClassAdjacent(MemberInfo))
	{
		ChildInfo->MemberNodItself = Twinkie->ParamGet<CMwNod*>(Nod, MemberInfo, IsNodImpersistent);
	}

	HasSetWindowPositionForPopup = false;
}

void NodExplorerModule::SetClassInfoPopup(CMwClassInfo* ClassInfo, const std::string NodName, CMwNod* Nod, bool IsNodImpersistent)
{
	// Fix memory leak incase of user clicking on another nod while pop-up is still active
	if (NodInfo)
	{
		delete NodInfo;
		NodInfo = nullptr;
	}

	if (ChildInfo)
	{
		delete ChildInfo;
		ChildInfo = nullptr;
		MemberInfoPopup = false;
	}

	NodInfoPopup = true;
	NodInfo = new PopupNodInfo{NodName};
	NodInfo->ClassInfo = ClassInfo;
	NodInfo->Nod = Nod;

	HasSetWindowPositionForPopup = false;
}

std::string NodExplorerModule::GetFancyMemberName(CMwMemberInfo* MemberInfo)
{
	std::string MemberNameFallback = MemberInfo->MemberName;

	if (MemberNameFallback.empty())
	{
		MemberNameFallback = std::format("__0x{:08x}", MemberInfo->MemberID);
	}

	std::string FancyMemberName = std::format("{} (+0x{:x})",
		MemberNameFallback,
		(uint32_t)MemberInfo->MemberOffset
	);

	return FancyMemberName;
}

void NodExplorerModule::RenderNod(CMwNod* Nod, const std::string NodName, CMwMemberInfo* NodMemberInfo = nullptr)
{
	bool IsNodVirtual = false;
	if (NodMemberInfo) IsNodVirtual = NodMemberInfo->MemberOffset == 0xFFFFFFFFU;

	bool IsNodImpersistent = false;
#ifdef TMCN
	if (NodMemberInfo) IsNodImpersistent = NodMemberInfo->MemberType == CMwMemberInfo::CLASSNOTPERSISTENT;
#endif

	CMwMemberInfoClass* MemberInfoAsClass = (CMwMemberInfoClass*)NodMemberInfo;
	if (!Nod)
	{
		BeginDisabled();
		if (NodMemberInfo)
		{
			Text("%s* %s", MemberInfoAsClass->ClassInfo->ClassName, NodName.c_str());
		}
		else
		{
			Text(NodName.c_str());
		}
		EndDisabled();
		return;
	}

	CMwClassInfo* ClassInfo = IsNodImpersistent ? MemberInfoAsClass->ClassInfo : Nod->MwGetClassInfo();

	std::string NodAddressStr = std::format("{:p}", (void*)Nod);
	std::string ClassTypeName = std::format("{}*{}",
		ClassInfo->ClassName,
		MemberInfoAsClass and (MemberInfoAsClass->ClassInfo->ClassID != ClassInfo->ClassID) ? std::string(" $999(") + MemberInfoAsClass->ClassInfo->ClassName + "*)$z" : std::string("")
	); 
	std::string TreeNodeLabel = std::format("{}{} {} ({})",
		IsNodVirtual ? std::string("$66fV$z ") : std::string(""),
		ClassTypeName,
		NodName,
		NodAddressStr
	);

	PushID((void*)Nod);
	bool TreeNodeIsOpen = TreeNodeEx(TreeNodeLabel.c_str(), ImGuiTreeNodeFlags_DrawLinesFull);

	if (HasRightClickedOnItem())
	{
		SetClassInfoPopup(ClassInfo, NodName, Nod, IsNodImpersistent);
	}

	if (TreeNodeIsOpen)
	{
		while (ClassInfo)
		{
			PushID(ClassInfo);

			for (auto& MemberInfo : *ClassInfo)
			{
				std::string FancyMemberName = GetFancyMemberName(MemberInfo);

				bool IgnoreRightClicks = false;

				switch (MemberInfo->MemberType)
				{
#ifdef TMCN
					case CMwMemberInfo::PROC:
					{
						auto ParamToStr = [](CMwMemberInfoProc::Param& Param) {
							return std::format(
#ifdef _DEBUG
								"{} ({}, {}, {:x})", 
								Param.Name, 
								(uint32_t)Param.Type.Type, 
								Param.Type.AdditionalInfo < CMwMemberInfo::MAX ?
									g_MemberTypeNames[Param.Type.AdditionalInfo] :
									std::to_string   (Param.Type.AdditionalInfo)
								,
								(uint32_t)Param.Flags
#else
								"{} {}",
								Param.Type.AdditionalInfo < CMwMemberInfo::MAX ?
									g_MemberTypeSignatures[Param.Type.AdditionalInfo] :
									std::to_string(Param.Type.AdditionalInfo)
								,
								Param.Name
#endif
							);
						};

						CMwMemberInfoProc::Param* ReturnParam = nullptr;

						auto MemberInfoAsProc = (CMwMemberInfoProc*)MemberInfo;

						std::vector<CMwMemberInfoProc::Param> Params = MemberInfoAsProc->GetParams();

						std::string ActualProcParameters = "";

						for (auto& Param : Params)
						{
							if ((Param.Flags & 2) == 2 and not ReturnParam) ReturnParam = &Param;
							else
							{
								ActualProcParameters += ParamToStr(Param) + ", ";
							}
						}

						std::string CumulativeProcSignature = 
							ReturnParam ? 
								ReturnParam->Type.AdditionalInfo < CMwMemberInfo::MAX ? 
									(std::string)g_MemberTypeSignatures[ReturnParam->Type.AdditionalInfo] + " " :
									std::to_string(ReturnParam->Type.AdditionalInfo) + " " 
							: "void ";
						CumulativeProcSignature += MemberInfo->MemberName;
						CumulativeProcSignature += " (";
						CumulativeProcSignature += ActualProcParameters;
						// Remove extra ", "
						CumulativeProcSignature = CumulativeProcSignature.substr(0, CumulativeProcSignature.length() - min(ActualProcParameters.length(), 2));
						CumulativeProcSignature += ")";

						Text("PROC %s", CumulativeProcSignature.c_str());

						break;
					}
					case CMwMemberInfo::CLASSNOTPERSISTENT:
					case CMwMemberInfo::CLASSALT:
#endif
					case CMwMemberInfo::CLASS:
					{
						CMwNod* ChildNod = Twinkie->ParamGet<CMwNod*>(Nod, MemberInfo, IsNodImpersistent);

						RenderNod(ChildNod, std::format("{} (+0x{:x})", MemberInfo->MemberName, MemberInfo->MemberOffset), MemberInfo);

						break;
					}

					case CMwMemberInfo::ACTION:
					{
						const auto* MemberInfoAction = static_cast<const CMwMemberInfoAction*>(MemberInfo);
						if (Button(FancyMemberName.c_str()))
						{
							MemberInfoAction->Action(Nod);
						}
						break;
					}

					case CMwMemberInfo::CLASSBUFFER:
					{
						CFastBuffer<CMwNod*>* Array = Twinkie->ParamGet<CFastBuffer<CMwNod*>>(Nod, MemberInfo, IsNodImpersistent);
						if (Array)
						{
							PushID((void*)Array);

							CMwMemberInfoClassArray* MemberInfoArray = (CMwMemberInfoClassArray*)MemberInfo;
							CMwClassInfo* ArrayClassType = MemberInfoArray->ArrayClassInfo;

							std::string ElementNameSingular =
#ifndef TMU
								MemberInfoArray->ElementNameSingular ? MemberInfoArray->ElementNameSingular :
#endif
								MemberInfoArray->MemberName;
							std::string ArrayName = std::format("CFastBuffer<{}*> {}", ArrayClassType->ClassName, FancyMemberName);

							bool ArrayTreeNodeOpened = TreeNodeEx(ArrayName.c_str(), ImGuiTreeNodeFlags_DrawLinesFull);

							if (HasRightClickedOnItem())
							{
								SetMemberInfoPopup(MemberInfo, NodName, Nod, ClassInfo);
							}

							if (ArrayTreeNodeOpened)
							{
								for (auto& ArrayNod : *Array)
								{
									RenderNod(ArrayNod, ElementNameSingular);
								}
								TreePop();
							}
							PopID();
						}
						else
						{
							BeginDisabled();

							CMwMemberInfoClassArray* MemberInfoArray = (CMwMemberInfoClassArray*)MemberInfo;
							CMwClassInfo* ArrayClassType = MemberInfoArray->ArrayClassInfo;
							std::string ArrayName = std::format("CFastBuffer<{}*> {}", ArrayClassType->ClassName, FancyMemberName);
							
							Text(ArrayName.c_str());
							
							EndDisabled();
						}
						break;
					}
					case CMwMemberInfo::CLASSARRAY:
					{
						CFastArray<CMwNod*>* Array = Twinkie->ParamGet<CFastArray<CMwNod*>>(Nod, MemberInfo, IsNodImpersistent);
						if (Array)
						{
							PushID((void*)Array);

							CMwMemberInfoClassArray* MemberInfoArray = (CMwMemberInfoClassArray*)MemberInfo;
							CMwClassInfo* ArrayClassType = MemberInfoArray->ArrayClassInfo;

							std::string ElementNameSingular =
#ifndef TMU
								MemberInfoArray->ElementNameSingular ? MemberInfoArray->ElementNameSingular :
#endif
								MemberInfoArray->MemberName;
							std::string ArrayName = std::format("CFastArray<{}*> {}", ArrayClassType->ClassName, FancyMemberName);

							bool ArrayTreeNodeOpened = TreeNodeEx(ArrayName.c_str(), ImGuiTreeNodeFlags_DrawLinesFull);

							if (HasRightClickedOnItem())
							{
								SetMemberInfoPopup(MemberInfo, NodName, Nod, ClassInfo);
							}

							if (ArrayTreeNodeOpened)
							{
								for (auto& ArrayNod : *Array)
								{
									RenderNod(ArrayNod, ElementNameSingular);
								}
								TreePop();
							}
							PopID();
						}
						else
						{
							BeginDisabled();

							CMwMemberInfoClassArray* MemberInfoArray = (CMwMemberInfoClassArray*)MemberInfo;
							CMwClassInfo* ArrayClassType = MemberInfoArray->ArrayClassInfo;
							std::string ArrayName = std::format("CFastArray<{}*> {}", ArrayClassType->ClassName, FancyMemberName);

							Text(ArrayName.c_str());

							EndDisabled();
						}
						break;
					}
#ifndef TMCN
					case CMwMemberInfo::REALRANGE:
#endif
#ifdef TMU
					// oh god
					case CMwMemberInfo::IDBUFFERCAT:
#endif
					case CMwMemberInfo::REAL:
					{
						float* Float = Twinkie->ParamGet<float>(Nod, MemberInfo, IsNodImpersistent);
						if (!Float) goto RerenderMember;
						float FloatV = *Float;
						if (InputFloat(FancyMemberName.c_str(), &FloatV))
						{
							Twinkie->ParamSet(Nod, MemberInfo, &FloatV);
						}
						break;
					}
#ifdef TMCN
					case CMwMemberInfo::REALRANGE:
					{
						CMwMemberInfoRealRange* MemberAsRealRange = (CMwMemberInfoRealRange*)MemberInfo;
						float* Float = Twinkie->ParamGet<float>(Nod, MemberInfo, IsNodImpersistent);
						if (!Float) goto RerenderMember;
						float FloatV = *Float;
						if (SliderFloat(FancyMemberName.c_str(), &FloatV, MemberAsRealRange->ValueMin, MemberAsRealRange->ValueMax))
						{
							Twinkie->ParamSet(Nod, MemberInfo, &FloatV);
						}
						break;
					}

					case CMwMemberInfo::INTRANGE:
					{
						CMwMemberInfoIntRange* MemberAsIntRange = (CMwMemberInfoIntRange*)MemberInfo;
						int* Int = Twinkie->ParamGet<int>(Nod, MemberInfo, IsNodImpersistent);
						if (!Int) goto RerenderMember;
						int IntV = *Int;
						if (SliderInt(FancyMemberName.c_str(), &IntV, MemberAsIntRange->ValueMin, MemberAsIntRange->ValueMax))
						{
							Twinkie->ParamSet(Nod, MemberInfo, &IntV);
						}
						break;
					}

					case CMwMemberInfo::ENUM:
					{
						CMwMemberInfoEnum* MemberAsEnum = (CMwMemberInfoEnum*)MemberInfo;
						int* CurrentItem = Twinkie->ParamGet<int>(Nod, MemberInfo, IsNodImpersistent);
						if (!CurrentItem) goto RerenderMember;
						int CurrentItemV = *CurrentItem;
						if (Combo((MemberAsEnum->EnumTypeName + (" " + FancyMemberName)).c_str(), &CurrentItemV, MemberAsEnum->EnumValueNames, MemberAsEnum->EnumValueNamesLength))
						{
							Twinkie->ParamSet(Nod, MemberInfo, &CurrentItemV);
						}
						break;
					}
#endif

					case CMwMemberInfo::NATURAL:
					case CMwMemberInfo::INT:
#ifdef GAMEBOX
					case CMwMemberInfo::ENUM:
#endif
					{
						int* Int = Twinkie->ParamGet<int>(Nod, MemberInfo, IsNodImpersistent);
						if (!Int) goto RerenderMember;
						int IntV = *Int;
						if (InputInt((FancyMemberName + std::format(" {:x}", (uint64_t)(uintptr_t)(void*)Int)).c_str(), &IntV))
						{
							Twinkie->ParamSet(Nod, MemberInfo, &IntV);
						}
						break;
					}

					case CMwMemberInfo::BOOL:
					{
						bool* Bool = Twinkie->ParamGet<bool>(Nod, MemberInfo, IsNodImpersistent);
						if (!Bool) goto RerenderMember;
						bool BoolV = *Bool;
						if (Checkbox(FancyMemberName.c_str(), &BoolV))
						{
							Twinkie->ParamSet(Nod, MemberInfo, &BoolV);
						}
						break;
					}

					default:
					{
RerenderMember:
						Text("%s (%s %d)", FancyMemberName.c_str(), g_MemberTypeNames[MemberInfo->MemberType], MemberInfo->MemberType);
						break;
					}
				}
				if (HasRightClickedOnItem() and not IgnoreRightClicks)
				{
					SetMemberInfoPopup(MemberInfo, NodName, Nod, ClassInfo, IsNodImpersistent);
				}
			}
			
			if (ClassInfo->ParentClassInfo)
			{
				SeparatorText(ClassInfo->ParentClassInfo->ClassName);

				if (HasRightClickedOnItem())
				{
					SetClassInfoPopup(ClassInfo->ParentClassInfo, NodName, Nod, IsNodImpersistent);
				}
			}

			// This is to fix a nullptr exception when trying to access the classinfo for popup because we break by default when classinfo is null
			// Technically, this makes the "while (ClassInfo)" clause above useless (i.e. could be infinite loop), but I'd rather not :P
			if (!ClassInfo->ParentClassInfo)
			{
				PopID();
				break;
			}
			ClassInfo = ClassInfo->ParentClassInfo;

			PopID();
		}
		TreePop();
	}

	PopID();
}

void NodExplorerModule::RenderMenu()
{
	if (MenuItem("NodExplorer", nullptr, Visible))
	{
		Visible = !Visible;
	}
}

void NodExplorerModule::RenderNodInfoClassInfo(CMwClassInfo* ClassInfo)
{
	{
		std::string Copyable = std::format("{:p}", (void*)NodInfo->Nod);
		if (Selectable((std::string("Copy: ") + Copyable + " (nod ptr)").c_str()))
		{
			SetClipboardText(Copyable.c_str());
			NodInfoPopup = false;
		}
	}
	{
		std::string Copyable = ClassInfo->ClassName;
		if (Selectable((std::string("Copy: ") + Copyable).c_str()))
		{
			SetClipboardText(Copyable.c_str());
			NodInfoPopup = false;
		}
	}
	{
		std::string Copyable = std::format("{:x}", ClassInfo->ClassID);
		if (Selectable((std::string("Copy: ") + Copyable).c_str()))
		{
			SetClipboardText(Copyable.c_str());
			NodInfoPopup = false;
		}
	}
	{
		void* ClassInfoAddr = reinterpret_cast<void*>(ClassInfo);
		std::string Copyable = std::format("{:p}", ClassInfoAddr);
		if (Selectable((std::string("Copy: ") + Copyable + " (class info ptr)").c_str()))
		{
			SetClipboardText(Copyable.c_str());
			NodInfoPopup = false;
		}
	}
}

void NodExplorerModule::RenderNodInfoMemberInfo()
{
	CMwMemberInfo* MemberInfo = ChildInfo->MemberInfo;
	{
		std::string Copyable = MemberInfo->MemberName;
		if (Selectable((std::string("Copy: ") + Copyable).c_str()))
		{
			SetClipboardText(Copyable.c_str());
			NodInfoPopup = false;
		}
	}
	{
		std::string Copyable = std::format("{:x}", MemberInfo->MemberID);
		if (Selectable((std::string("Copy: ") + Copyable).c_str()))
		{
			SetClipboardText(Copyable.c_str());
			NodInfoPopup = false;
		}
	}
	{
		void* MemberInfoAddr = reinterpret_cast<void*>(MemberInfo);
		std::string Copyable = std::format("{:p}", MemberInfoAddr);
		if (Selectable((std::string("Copy: ") + Copyable + " (member info ptr)").c_str()))
		{
			SetClipboardText(Copyable.c_str());
			NodInfoPopup = false;
		}
	}
	{
		std::string Copyable = g_MemberTypeNames[(size_t)MemberInfo->MemberType];
		if (Selectable((std::string("Copy: ") + Copyable + "").c_str()))
		{
			SetClipboardText(Copyable.c_str());
			NodInfoPopup = false;
		}
	}
	if (ChildInfo->MemberNodItself)
	{
		Separator();

		CMwClassInfo* MemberNodClassInfo =
#ifdef TMCN
			nullptr;
		if (ChildInfo->MemberInfo->MemberType == CMwMemberInfo::CLASSNOTPERSISTENT)
		{
			MemberNodClassInfo = ((CMwMemberInfoClass*)ChildInfo->MemberInfo)->ClassInfo;
		}
		else
		{
			MemberNodClassInfo = 
#endif
				ChildInfo->MemberNodItself->MwGetClassInfo();
#ifdef TMCN
		}
#endif
		RenderNodInfoClassInfo(MemberNodClassInfo);
	}
}

void NodExplorerModule::RenderInterface()
{
	static struct
	{
		uintptr_t Addr = 0;
		bool IsSafe = false;
	} OtherNod;

	if (!Visible) return;

	Begin("NodExplorer");

	BeginTabBar("NodExplorerTabBar");

	if (BeginTabItem("App", nullptr, ImGuiTabItemFlags_NoCloseWithMiddleMouseButton))
	{
		RenderNod(Twinkie->GetApp(), "App");
		EndTabItem();
	}
	
	if (BeginTabItem("Other", nullptr, ImGuiTabItemFlags_NoCloseWithMiddleMouseButton))
	{
		static uint64_t Temp = 0;
		InputScalar("Address", ImGuiDataType_U64, &Temp, nullptr, nullptr, "%x");
		SameLine();
		if (Button(">"))
		{
			bool PtrSafe = false;
			bool VftableSafe = false;

			PtrSafe = IsAddrSafeSlow(Temp);
			if (PtrSafe) VftableSafe = IsAddrSafeSlow(*(uintptr_t*)Temp);

			bool IsSafe = PtrSafe and VftableSafe;

			if (OtherNod.IsSafe and not IsSafe)
			{
				// do nothing...
			}
			else if (IsSafe)
			{
				OtherNod.Addr = Temp;
				OtherNod.IsSafe = IsSafe;
			}
		}

		if (OtherNod.IsSafe)
		{
			RenderNod((CMwNod*)OtherNod.Addr, "Other");
		}

		EndTabItem();
	}

	EndTabBar();

	End();

	if (NodInfoPopup)
	{
		Begin("##NodExplorerNodInfoPopup", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);

		if (!IsWindowFocused())
		{
			NodInfoPopup = false;
			if (NodInfo)
			{
				delete NodInfo;
				NodInfo = nullptr;

				End();
				goto ContinueAfterRender;
			}
		}
		else if (!HasSetWindowPositionForPopup)
		{
			HasSetWindowPositionForPopup = true;
			SetWindowPos(GetMousePos());
		}
		
		if (NodInfo)
		{
			std::string Copyable = NodInfo->Name;
			if (Selectable((std::string("Copy: ") + Copyable).c_str()))
			{
				SetClipboardText(Copyable.c_str());
				NodInfoPopup = false;
			}

			RenderNodInfoClassInfo(NodInfo->ClassInfo);
		}
		

		End();
	}
	else if (MemberInfoPopup)
	{
		Begin("##NodExplorerMemberInfoPopup", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);

		if (!IsWindowFocused())
		{
			MemberInfoPopup = false;
			if (ChildInfo)
			{
				delete ChildInfo;
				ChildInfo = nullptr;

				End();
				goto ContinueAfterRender;
			}
		}
		else if (!HasSetWindowPositionForPopup)
		{
			HasSetWindowPositionForPopup = true;
			SetWindowPos(GetMousePos());
		}

		RenderNodInfoMemberInfo();

		End();
	}

ContinueAfterRender:
	if (HasSetWindowPositionForPopup and (!NodInfoPopup and !MemberInfoPopup))
	{
		HasSetWindowPositionForPopup = false;
	}
	return;
}