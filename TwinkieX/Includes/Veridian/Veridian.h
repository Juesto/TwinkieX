#pragma once

#pragma warning(disable : 4201 4583 5267 4625)
#pragma warning(push)

#include <imgui/imgui.h>
#include <cstdint>
#include <string>
#include <map>
#include <filesystem>
#include <fstream>
#include <format>
#include <iostream>
#include <functional>

// For befriending with VSetCtx
class TwinkUi;

#define VERIDIAN_FALLBACK_SECTION_NAME "FallbackSection"

// The veridian settings manager
namespace Veridian
{
    struct VVec2
    {
        VVec2(std::string String)
        {
            if (String.find_first_of(',') != std::string::npos)
            {
                size_t CommaPos = String.find_first_of(',');
                std::string First = String.substr(0, CommaPos);
                std::string Second = String.substr(CommaPos + 1);

                x = std::stof(First);
                y = std::stof(Second);
            }
        }
        VVec2(float x = 0.f, float y = 0.f) : x(x), y(y) {}

        float x, y;

        operator ImVec2()
        {
            return ImVec2(x, y);
        }
    };

    struct VVec3
    {
        VVec3(std::string String)
        {
            size_t FirstCommaPos = String.find(',');
            size_t SecondCommaPos = String.find(',', FirstCommaPos + 1);

            if (FirstCommaPos != std::string::npos &&
                SecondCommaPos != std::string::npos)
            {
                std::string First = String.substr(0, FirstCommaPos);
                std::string Second = String.substr(FirstCommaPos + 1,
                    SecondCommaPos - FirstCommaPos - 1);
                std::string Third = String.substr(SecondCommaPos + 1);

                x = std::stof(First);
                y = std::stof(Second);
                z = std::stof(Third);
            }
        }
        VVec3(float x = 0.f, float y = 0.f, float z = 0.f) : x(x), y(y), z(z) {}

        union
        {
            struct
            {
                float x, y, z;
            };
            struct
            {
                float r, g, b;
            };
        };

        operator ImVec4()
        {
            return ImVec4(x, y, z, 1.f);
        }
    };

    struct VVec4
    {
        VVec4(std::string String)
        {
            size_t FirstCommaPos = String.find_first_of(',');
            size_t SecondCommaPos = String.find(',', FirstCommaPos + 1);
            size_t ThirdCommaPos = String.find(',', SecondCommaPos + 1);

            if (FirstCommaPos != std::string::npos &&
                SecondCommaPos != std::string::npos &&
                ThirdCommaPos != std::string::npos)
            {
                std::string First = String.substr(0, FirstCommaPos);
                std::string Second = String.substr(FirstCommaPos + 1,
                    SecondCommaPos - FirstCommaPos - 1);
                std::string Third = String.substr(SecondCommaPos + 1,
                    ThirdCommaPos - SecondCommaPos - 1);
                std::string Fourth = String.substr(ThirdCommaPos + 1);

                x = std::stof(First);
                y = std::stof(Second);
                z = std::stof(Third);
                w = std::stof(Fourth);
            }
        }
        VVec4(float x = 0.f, float y = 0.f, float z = 0.f, float w = 0.f) : x(x), y(y), z(z), w(w) {}

        union
        {
            struct
            {
                float x, y, z, w;
            };
            struct
            {
                float r, g, b, a;
            };
        };

        operator ImVec4()
        {
            return ImVec4(x, y, z, w);
        }
    };

    enum class VSettingType
    {
        VUnknown = 0,
        // 64-bit integer
        VInt,
        // 64-bit unsigned integer
        VUInt,
        VFloat,
        VString,
        VVec2,
        VVec3,
        VVec4,
        VBool,
    };

    extern std::unordered_map<VSettingType, size_t> TypeToSize;

    union VValue
    {
        int64_t Int;
        uint64_t UInt;
        float Float;
        std::string String;
        VVec2 Vec2;
        VVec3 Vec3;
        VVec4 Vec4;
        bool Bool;

        ~VValue()
        {
        }

        VValue()
        {
            memset(this, 0, sizeof(VValue));
        }
        VValue(VValue& Other)
        {
            memcpy_s(this, sizeof(VValue), &Other, sizeof(VValue));
        }
        VValue operator=(VValue& Other)
        {
            memcpy_s(this, sizeof(VValue), &Other, sizeof(VValue));

            return *this;
        }

        template <typename T>
        VValue operator=(T Value)
        {
            if constexpr (std::is_same_v<T, int64_t>)
            {
                Int = Value;
            }
            else if constexpr (std::is_same_v<T, uint64_t>)
            {
                UInt = Value;
            }
            else if constexpr (std::is_same_v<T, float>)
            {
                Float = Value;
            }
            else if constexpr (std::is_same_v<T, std::string> or std::is_same_v<T, std::string&>)
            {
                String = Value;
            }
            else if constexpr (std::is_same_v<T, VVec2> or std::is_same_v<T, ImVec2>)
            {
                Vec2 = Value;
            }
            else if constexpr (std::is_same_v<T, VVec3>)
            {
                Vec3 = Value;
            }
            else if constexpr (std::is_same_v<T, VVec4>)
            {
                Vec4 = Value;
            }
            else if constexpr (std::is_same_v<T, ImVec4>)
            {
                throw std::exception("using ImVec4 in VSettting.Set is ambiguous; consider using VVec3 or VVec4");
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
                Bool = Value;
            }
            else
            {
                throw std::exception("unknown type");
            }
        }

        template <typename T>
        operator T()
        {
            if constexpr (std::is_same_v<T, int64_t>)
            {
                return Int;
            }
            else if constexpr (std::is_same_v<T, uint64_t>)
            {
                return UInt;
            }
            else if constexpr (std::is_same_v<T, float>)
            {
                return Float;
            }
            else if constexpr (std::is_same_v<T, std::string> or std::is_same_v<T, std::string&>)
            {
                return String;
            }
            else if constexpr (std::is_same_v<T, VVec2> or std::is_same_v<T, ImVec2>)
            {
                return Vec2;
            }
            else if constexpr (std::is_same_v<T, VVec3>)
            {
                return Vec3;
            }
            else if constexpr (std::is_same_v<T, VVec4>)
            {
                return Vec4;
            }
            else if constexpr (std::is_same_v<T, ImVec4>)
            {
                throw std::exception("using ImVec4 in VSettting.Get is ambiguous; consider using VVec3 or VVec4");
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
                return Bool;
            }
            else
            {
                throw std::exception("unknown type");
            }
        }
    };

    class VSetting;
    using VSettingRenderFn = std::function<bool(VSetting*)>;

    struct VSettingRenderFnStruct
    {
        VSettingRenderFn BeforeRender = nullptr;
        VSettingRenderFn Render = nullptr;
        VSettingRenderFn AfterRender = nullptr;
    };

    class VSetting
    {
        friend class VSetCtx;
        friend struct VSettingSorter;

        std::string SettingStr;
        size_t Idx;

    public:
        VSetting()
        {

        }
        VSetting operator=(VSetting Other)
        {
            this->Type = Other.Type;
            this->Name = Other.Name;
            this->FacingName = Other.FacingName;
            this->Section = Other.Section;
            this->Value = Other.Value;

            return *this;
        }

        bool Registered = false;
        bool Hidden = false;

        VSettingType Type = VSettingType::VUnknown;

        std::string Name;
        std::string FacingName;
        std::string Section;
        VValue* Value = nullptr;
        VSettingRenderFnStruct ExtraRenderFn{};

        template <typename T> T Get()
        {
            if constexpr (std::is_same_v<T, int64_t>)
            {
                return this->Value->Int;
            }
            else if constexpr (std::is_same_v<T, uint64_t>)
            {
                return this->Value->UInt;
            }
            else if constexpr (std::is_same_v<T, float>)
            {
                return this->Value->Float;
            }
            else if constexpr (std::is_same_v<T, std::string> or std::is_same_v<T, std::string&>)
            {
                return this->Value->String;
            }
            else if constexpr (std::is_same_v<T, ::VVec2> or std::is_same_v<T, ImVec2>)
            {
                return this->Value->Vec2;
            }
            else if constexpr (std::is_same_v<T, ::VVec3>)
            {
                return this->Value->Vec3;
            }
            else if constexpr (std::is_same_v<T, ::VVec4>)
            {
                return this->Value->Vec4;
            }
            else if constexpr (std::is_same_v<T, ImVec4>)
            {
                throw std::exception("using ImVec4 in VSettting.Get is ambiguous; consider using VVec3 or VVec4");
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
                return this->Value->Bool;
            }
            else
            {
                throw std::exception("unknown type");
            }
        }

        template <typename T> void Set(T Value)
        {
            if constexpr (std::is_same_v<T, int64_t>)
            {
                this->Value->Int = Value;
            }
            else if constexpr (std::is_same_v<T, uint64_t>)
            {
                this->Value->UInt = Value;
            }
            else if constexpr (std::is_same_v<T, float>)
            {
                this->Value->Float = Value;
            }
            else if constexpr (std::is_same_v<T, std::string> or std::is_same_v<T, std::string&> or std::is_same_v<T, const char*> or std::is_same_v<T, char*>)
            {
                this->Value->String = Value;
            }
            else if constexpr (std::is_same_v<T, ::VVec2> or std::is_same_v<T, ImVec2>)
            {
                this->Value->Vec2 = Value;
            }
            else if constexpr (std::is_same_v<T, ::VVec3>)
            {
                this->Value->Vec3 = Value;
            }
            else if constexpr (std::is_same_v<T, ::VVec4>)
            {
                this->Value->Vec4 = Value;
            }
            else if constexpr (std::is_same_v<T, ImVec4>)
            {
                throw std::exception("using ImVec4 in VSettting.Set is ambiguous; consider using VVec3 or VVec4");
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
                this->Value->Bool = Value;
            }
            else
            {
                throw std::exception("unknown type");
            }

            this->SettingStr = GetAsString();
        }

        std::string GetAsString()
        {
            if (not Value) return SettingStr;

            switch (Type)
            {
            case VSettingType::VInt:
            {
                return std::to_string(Value->Int);
                break;
            }
            case VSettingType::VUInt:
            {
                return std::to_string(Value->UInt);
                break;
            }
            case VSettingType::VFloat:
            {
                return std::to_string(Value->Float);
                break;
            }
            case VSettingType::VString:
            {
                return Value->String;
                break;
            }
            case VSettingType::VVec2:
            {
                return std::format("{},{}", Value->Vec2.x, Value->Vec2.y);
                break;
            }
            case VSettingType::VVec3:
            {
                return std::format("{},{},{}", Value->Vec3.x, Value->Vec3.y, Value->Vec3.z);
                break;
            }
            case VSettingType::VVec4:
            {
                return std::format("{},{},{},{}", Value->Vec4.x, Value->Vec4.y, Value->Vec4.z, Value->Vec4.w);
                break;
            }
            case VSettingType::VBool:
            {
                return Value->Bool ? "true" : "false";
                break;
            }
            default:
            {
                throw std::exception("unknown type");
                break;
            }
            }
        }

        private:
        void SetFromString()
        {
            VValue* NewValue = new VValue();
            Value = NewValue;

            try
            {
                switch (Type)
                {
                case VSettingType::VInt:
                {
                    Value->Int = std::stoll(SettingStr);
                    break;
                }
                case VSettingType::VUInt:
                {
                    Value->UInt = std::stoull(SettingStr);
                    break;
                }
                case VSettingType::VFloat:
                {
                    Value->Float = std::stof(SettingStr);
                    break;
                }
                case VSettingType::VString:
                {
                    delete Value;
                    Value = (VValue*) & SettingStr;
                    break;
                }
                case VSettingType::VVec2:
                {
                    Value->Vec2 = VVec2(SettingStr);
                    break;
                }
                case VSettingType::VVec3:
                {
                    Value->Vec3 = VVec3(SettingStr);
                    break;
                }
                case VSettingType::VVec4:
                {
                    Value->Vec4 = VVec4(SettingStr);
                    break;
                }
                case VSettingType::VBool:
                {
                    Value->Bool = SettingStr == "true" ? true : false;
                    break;
                }
                default:
                {
                    delete Value;
                    Value = nullptr;
                    break;
                }
                }
            }
            catch (std::exception&)
            {
                // ...
            }
        }
    };
    class VSetCtx
    {
    public:
        friend class ::TwinkUi;

        VSetCtx() = delete;
        VSetCtx(std::filesystem::path Filepath);
        VSetCtx(VSetCtx&) = default;
        VSetCtx(VSetCtx&&) = default;
        VSetCtx operator=(VSetCtx&) = delete;
        VSetCtx operator=(VSetCtx&&) = delete;
        ~VSetCtx();

        struct Inner
        {
            std::unordered_map<std::string, VSetting> Values;
            std::vector<std::string> Order;
        };

        std::unordered_map<std::string, Inner> SettingValues;
        std::vector<std::string> SettingOrder;
        std::filesystem::path Filepath;

        VSetting& Register(std::string Section, std::string Name, std::string FacingName, VSettingType Type, void* Value, bool Hidden = false, VSettingRenderFnStruct ExtraRenderFn = {});

    private:
        std::fstream FileStream;
        
        void PrepareForDestruction();
    };

    // The vast (global) veridian settings manager
    // NOTE: Cornball
    extern VSetCtx* VastVeridian;

    void InitContext(std::filesystem::path Filepath);

    VSetting& Register(std::string Section, std::string Name, std::string FacingName, VSettingType Type, void* Value, bool Hidden = false, VSettingRenderFnStruct ExtraRenderFn = {});

    void RenderSetting(VSetting& Setting);
    void RenderSetting(std::string Section, std::string Name);
    void RenderSection(std::string Section);
    void RenderAll(std::string DefaultSectionName);
}

#pragma warning(pop)
