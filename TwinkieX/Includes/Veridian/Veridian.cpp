#include "pch.h"
#include "Veridian.h"
#include "Twinkie/TwinkUi/TwinkUi.h"

namespace Veridian
{
    VSetCtx* VastVeridian = nullptr;

    std::unordered_map<VSettingType, size_t> TypeToSize = {
        {VSettingType::VInt, sizeof(int64_t)},
        {VSettingType::VUInt, sizeof(uint64_t)},
        {VSettingType::VFloat, sizeof(float)},
        {VSettingType::VString, sizeof(std::string)},
        {VSettingType::VVec2, sizeof(VVec2)},
        {VSettingType::VVec3, sizeof(VVec3)},
        {VSettingType::VVec4, sizeof(VVec4)},
        {VSettingType::VBool, sizeof(bool)},
        {VSettingType::VUnknown, (size_t)-1LL}
    };

    VSetCtx::VSetCtx(std::filesystem::path Filepath)
    {
        this->Filepath = Filepath;

        if (not std::filesystem::exists(Filepath))
        {
            auto Filename = Filepath.filename();

            Filepath.remove_filename();
            std::filesystem::create_directories(Filepath);

            Filepath /= Filename;

            std::fstream(Filepath.wstring(), std::ios::in | std::ios::out | std::ios::trunc).close();
        }

        FileStream.open(Filepath.wstring(), std::ios::in | std::ios::out);

        if (not FileStream.is_open())
        {
            throw;
        }

        std::string Line;
        std::string CurrentSector = VERIDIAN_FALLBACK_SECTION_NAME;

        while (std::getline(FileStream, Line))
        {
            if (Line.starts_with("["))
            {
                CurrentSector = Line.substr(1, Line.length() - 2);
                continue;
            }
            else if (Line.find("=") != std::string::npos)
            {
                size_t EqualsSign = Line.find("=");

                std::string Name = Line.substr(0, EqualsSign);
                std::string Result = Line.substr(EqualsSign + 1);

                SettingValues[CurrentSector].Values[Name] = VSetting();

                VSetting& NewSetting = SettingValues[CurrentSector].Values[Name];

                NewSetting.Name = Name;
                NewSetting.SettingStr = Result;
            }
        }
    }

    VSetCtx::~VSetCtx()
    {
        FileStream.clear();
        FileStream.seekp(0);

        for (auto& Sector : SettingValues)
        {
            FileStream << "[" << Sector.first << "]\n";

            for (auto& Setting : SettingValues[Sector.first].Order)
            {
                FileStream << SettingValues[Sector.first].Values[Setting].Name << "=" << SettingValues[Sector.first].Values[Setting].GetAsString() << "\n";
            }

            FileStream << "\n";
        }

        FileStream.close();
    }

    VSetting& VSetCtx::Register(std::string Section, std::string Name, std::string FacingName, VSettingType Type, void* Value, bool Hidden, VSettingRenderFnStruct ExtraRenderFn)
    {
        if (this->SettingValues.contains(Section))
        {
            if (this->SettingValues[Section].Values.contains(Name))
            {
                VSetting& ExistingSetting = this->SettingValues[Section].Values[Name];
                ExistingSetting.Type = Type;
                ExistingSetting.SetFromString();

                if (Value and ExistingSetting.Value)
                {
                    if (Type == VSettingType::VString)
                        *reinterpret_cast<std::string*>(Value) = ExistingSetting.SettingStr;
                    else
                        memcpy_s(Value, TypeToSize[Type], ExistingSetting.Value, TypeToSize[Type]);
                }

                if (Type != VSettingType::VString) delete ExistingSetting.Value;
                ExistingSetting.Value = nullptr;
            }
        }

        if (std::find(SettingOrder.begin(), SettingOrder.end(), Section) == SettingOrder.end()) SettingOrder.push_back(Section);
        if (std::find(SettingValues[Section].Order.begin(), SettingValues[Section].Order.end(), Name) == SettingValues[Section].Order.end())
            SettingValues[Section].Order.push_back(Name);

        VSetting& NewSettingRef = this->SettingValues[Section].Values[Name];

        NewSettingRef.Name = Name;
        NewSettingRef.FacingName = FacingName;
        NewSettingRef.Section = Section;
        NewSettingRef.Type = Type;
        NewSettingRef.Registered = true;
        NewSettingRef.Hidden = Hidden;
        NewSettingRef.ExtraRenderFn = ExtraRenderFn;
        NewSettingRef.Value = reinterpret_cast<VValue*>(Value);

        return this->SettingValues[Section].Values[Name];
    }

    void VSetCtx::PrepareForDestruction()
    {
        for (auto& SectionName : this->SettingOrder)
        {
            auto& Section = this->SettingValues[SectionName];
            for (auto& SettingName : Section.Order)
            {
                auto& Setting = Section.Values[SettingName];

                if (Setting.Value)
                {
                    Setting.SettingStr = Setting.GetAsString();
                    Setting.Value = nullptr;
                }
            }
        }
    }

    void InitContext(std::filesystem::path Filepath)
    {
        VastVeridian = new VSetCtx(Filepath);
    }

    void RenderSetting(VSetting& Setting)
    {
        if (Setting.Hidden) return;

        if (Setting.ExtraRenderFn.BeforeRender)
        {
            Setting.ExtraRenderFn.BeforeRender(&Setting);
        }

        static std::map<VSettingType, ImGuiDataType> VeridianToImGui =
        {
            {VSettingType::VInt, ImGuiDataType_S64},
            {VSettingType::VUInt, ImGuiDataType_U64},
            {VSettingType::VFloat, ImGuiDataType_Float}
        };

        char TempBuffer[513] = {};

        if (Setting.Value != nullptr and not Setting.ExtraRenderFn.Render)
        {
            if (Setting.Type == VSettingType::VString)
            {
                strcpy_s(TempBuffer, 513, Setting.Value->String.c_str());
            }

            switch (Setting.Type)
            {
            case VSettingType::VUInt:
            case VSettingType::VInt:
            case VSettingType::VFloat:
                ImGui::InputScalar(Setting.FacingName.c_str(), VeridianToImGui[Setting.Type], Setting.Value);
                break;
            case VSettingType::VBool:
                ImGui::Checkbox(Setting.FacingName.c_str(), &Setting.Value->Bool);
                break;
            case VSettingType::VVec2:
                ImGui::InputFloat2(Setting.FacingName.c_str(), &Setting.Value->Float);
                break;
            case VSettingType::VVec3:
                ImGui::ColorEdit3(Setting.FacingName.c_str(), &Setting.Value->Float);
                break;
            case VSettingType::VVec4:
                ImGui::ColorEdit4(Setting.FacingName.c_str(), &Setting.Value->Float);
                break;
            case VSettingType::VString:
                ImGui::InputText(Setting.FacingName.c_str(), TempBuffer, 513);
                break;
            default:
                ImGui::Text("Error rending setting %s (in %s): unknown type", Setting.Name.c_str(), Setting.Section.c_str());
                break;
            }
        }
        else if (Setting.Value == nullptr)
        {
            static int64_t Int = 0;
            static uint64_t UInt = 0;
            static float Float = 0;
            static bool Bool = false;
            static VVec2 Vec2 = VVec2();
            static VVec3 Vec3 = VVec3();
            static VVec4 Vec4 = VVec4();
            static std::string String;

            strcpy_s(TempBuffer, 513, String.c_str());

            ImGui::BeginDisabled();

            switch (Setting.Type)
            {
            case VSettingType::VUInt:
            case VSettingType::VInt:
            case VSettingType::VFloat:
                ImGui::InputScalar(Setting.FacingName.c_str(), VeridianToImGui[Setting.Type], &Float);
                break;
            case VSettingType::VBool:
                ImGui::Checkbox(Setting.FacingName.c_str(), &Bool);
                break;
            case VSettingType::VVec2:
                ImGui::InputFloat2(Setting.FacingName.c_str(), &Vec2.x);
                break;
            case VSettingType::VVec3:
                ImGui::ColorEdit3(Setting.FacingName.c_str(), &Vec3.x);
                break;
            case VSettingType::VVec4:
                ImGui::ColorEdit4(Setting.FacingName.c_str(), &Vec4.x);
                break;
            case VSettingType::VString:
                ImGui::InputText(Setting.FacingName.c_str(), TempBuffer, 513);
                break;
            default:
                ImGui::Text("Error rending setting %s (in %s): unknown type", Setting.Name.c_str(), Setting.Section.c_str());
                break;
            }

            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            {
                ImGui::SetTooltip("Item has been disabled because it has not been registered properly.");
            }

            ImGui::EndDisabled();
        }
        else if (Setting.ExtraRenderFn.Render)
        {
            Setting.ExtraRenderFn.Render(&Setting);
        }

        if (Setting.ExtraRenderFn.AfterRender)
        {
            Setting.ExtraRenderFn.AfterRender(&Setting);
        }
    }

    void RenderSetting(std::string Section, std::string Name)
    {
        if (not VastVeridian->SettingValues.contains(Section)) return;
        if (not VastVeridian->SettingValues[Section].Values.contains(Name)) return;

        RenderSetting(VastVeridian->SettingValues[Section].Values[Name]);
    }

    void RenderSection(std::string Section)
    {
        if (not VastVeridian->SettingValues.contains(Section)) return;

        for (auto& SetPair : VastVeridian->SettingValues.at(Section).Order)
        {
            if (VastVeridian->SettingValues[Section].Values[SetPair].Registered) RenderSetting(VastVeridian->SettingValues[Section].Values[SetPair]);
        }
    }

    void RenderAll(std::string DefaultSectionName)
    {
        static std::string ActiveSection = DefaultSectionName;

        std::map<std::string, size_t> RegisteredSettingsPerSection = {};

        for (auto& Section : VastVeridian->SettingOrder)
        {
            for (auto& Setting : VastVeridian->SettingValues[Section].Order)
            {
                if (VastVeridian->SettingValues[Section].Values[Setting].Registered and not VastVeridian->SettingValues[Section].Values[Setting].Hidden)
                {
                    RegisteredSettingsPerSection[Section]++;
                }
            }
        }

        if (ImGui::BeginChild("##SettingsSidebar", { 150.f, 0.f }, ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX))
        {
            if (ImGui::Selectable(DefaultSectionName.c_str(), DefaultSectionName == ActiveSection))
            {
                ActiveSection = DefaultSectionName;
            }

            for (auto& Section : VastVeridian->SettingOrder)
            {
                if (RegisteredSettingsPerSection[Section] == 0) continue;

                if (Section == DefaultSectionName) continue;

                if (ImGui::Selectable(Section.c_str(), Section == ActiveSection))
                {
                    ActiveSection = Section;
                }
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginGroup();

        RenderSection(ActiveSection);

        ImGui::EndGroup();
    }

    VSetting& Register(std::string Section, std::string Name, std::string FacingName, VSettingType Type, void* Value, bool Hidden, VSettingRenderFnStruct ExtraRenderFn)
    {
        return VastVeridian->Register(Section, Name, FacingName, Type, Value, Hidden, ExtraRenderFn);
    }
}
