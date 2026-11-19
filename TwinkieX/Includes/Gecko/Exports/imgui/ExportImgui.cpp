
#include "pch.h"
#include <Gecko/Exports/GeckoAutoExport.h>
#include "ExportImgui.h"
#include "Texture/Texture.h"
#include <Gecko/Exports/Global/MemoryBuffer/MemoryBuffer.h>

#define Gecko_Exports_ImGui_GuardNonRender(...) if (!Gecko::TheScriptManager->IsExecutingRenderCallback()) /* TODO: Throw an error */ { (void)0; return __VA_ARGS__; } (void)0

namespace Gecko::Exports::ImGui
{
	using namespace ::ImGui;

	static std::vector<GeckoExport> s_Exports;
#define UI_EXPORT(Decl, Fn) GECKO_EXPORT_FN(s_Exports, "UI", Decl, Fn)
#define Gecko_Exports_ImGui_MakeEndFn(Fn) void End##Fn##As() { \
			Gecko_Exports_ImGui_GuardNonRender(); \
			End##Fn(); \
		} UI_EXPORT("void End" #Fn "()", End##Fn##As)
#define Gecko_Exports_ImGui_MakeTrivialFn(Fn) void Fn##As() { \
			Gecko_Exports_ImGui_GuardNonRender(); \
			Fn(); \
		} UI_EXPORT("void " #Fn "()", Fn##As)
#define Gecko_Exports_ImGui_MakeTrivialReturnFn(Fn, ReturnType) decltype(Fn()) Fn##As() { \
			Gecko_Exports_ImGui_GuardNonRender(decltype(Fn()){}); \
			return Fn(); \
		} UI_EXPORT(#ReturnType " " #Fn "()", Fn##As)

	// UITexture
	namespace
	{
		/*
		!CompatMeta
		for     UI
		with    Texture@ LoadTexture(const string&in filename)
		as      Texture@ LoadTexture(const string&in filename)
		being   function
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		Texture* LoadTextureAs_File(std::u8string& FileName)
		{
			Texture* Tex = new Texture();
			if (!Tex->From(std::filesystem::path(FileName)))
			{
				delete Tex;
				return nullptr;
			}
			return Tex;
		}
		UI_EXPORT("Texture@+ LoadTexture(const string&in filename)", LoadTextureAs_File);

		/*
		!CompatMeta
		for     UI
		with    Texture@ LoadTexture(MemoryBuffer&in buffer)
		as      Texture@ LoadTexture(MemoryBuffer&in buffer)
		being   function
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		Texture* LoadTextureAs_Buffer(MemoryBuffer& Buffer)
		{
			if (!Buffer.GetSize()) return nullptr;

			uint64_t Position = Buffer.GetPosition();
			Buffer.Seek(0);
			std::u8string Bytes = Buffer.ReadString(Buffer.GetSize());
			Buffer.Seek(Position);

			Texture* Tex = new Texture();
			if (!Tex->From(Bytes.data(), Bytes.size()))
			{
				delete Tex;
				return nullptr;
			}
			return Tex;
		}
		UI_EXPORT("Texture@+ LoadTexture(MemoryBuffer&in buffer)", LoadTextureAs_Buffer);
	}

	// Begin/End functions
	namespace
	{
		/*
		!CompatMeta
		for     UI
		with    bool Begin(const string&in title, int flags = UI::GetDefaultWindowFlags())
		as      bool Begin(const string&in title, int flags)
		being   function
		declare partial
		since   v0.1.1
		reason  Flags are not pulled from any function before use.
		!CompatMeta
		*/
		bool BeginAs_NoOpen(std::u8string& title, int flags)
		{
			Gecko_Exports_ImGui_GuardNonRender(false);
			return Begin((const char*)title.c_str(), nullptr, flags);
		}
		UI_EXPORT("bool Begin(const string&in title, int flags = 0)", BeginAs_NoOpen);

		/*
		!CompatMeta
		for     UI
		with    bool Begin(const string&in title, bool&out open, int flags = UI::GetDefaultWindowFlags())
		as      bool Begin(const string&in title, bool&out open, int flags)
		being   function
		declare partial
		since   v0.1.1
		reason  Flags are not pulled from any function before use.
		!CompatMeta
		*/
		bool BeginAs_Open(std::u8string& title, bool* open, int flags)
		{
			Gecko_Exports_ImGui_GuardNonRender(false);
			return Begin((const char*)title.c_str(), open, flags);
		}
		UI_EXPORT("bool Begin(const string&in title, bool&out open, int flags = 0)", BeginAs_Open);

		/*
		!CompatMeta
		for     UI
		with    void End()
		as      void End()
		being   function
		declare full
		since   v0.1.0
		!CompatMeta
		*/
		Gecko_Exports_ImGui_MakeTrivialFn(End);

		/*
		!CompatMeta
		for     UI
		with    bool BeginCombo(const string&in label, const string&in current, int flags = UI::ComboFlags::None)
		as      bool BeginCombo(const string&in label, const string&in current, int flags = 0)
		being   function
		declare partial
		reason  UI::ComboFlags is not implemented.
		since   v0.1.1
		!CompatMeta
		*/
		bool BeginComboAs(std::u8string& label, std::u8string& current, int flags)
		{
			Gecko_Exports_ImGui_GuardNonRender(false);
			return BeginCombo((const char*)label.c_str(), (const char*)current.c_str(), flags);
		}
		UI_EXPORT("bool BeginCombo(const string&in label, const string&in current, int flags = 0)", BeginComboAs);

		/*
		!CompatMeta
		for     UI
		with    void EndCombo()
		as      void EndCombo()
		being   function
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		Gecko_Exports_ImGui_MakeEndFn(Combo);

		/*
		!CompatMeta
		for     UI
		with    void BeginDisabled(bool disabled = true)
		as      void BeginDisabled(bool disabled = true)
		being   function
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		void BeginDisabledAs(bool disabled)
		{
			Gecko_Exports_ImGui_GuardNonRender();
			BeginDisabled(disabled);
		}
		UI_EXPORT("void BeginDisabled(bool disabled = true)", BeginDisabledAs);

		/*
		!CompatMeta
		for     UI
		with    void EndDisabled()
		as      void EndDisabled()
		being   function
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		Gecko_Exports_ImGui_MakeEndFn(Disabled);

		/*
		!CompatMeta
		for     UI
		with    void BeginTabBar(const string&in id, int flags = UI::TabBarFlags::None)
		as      void BeginTabBar(const string&in id, int flags)
		being   function
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		void BeginTabBarAs(std::u8string& id, int flags)
		{
			Gecko_Exports_ImGui_GuardNonRender();
			BeginTabBar((const char*)id.c_str(), flags);
		}
		UI_EXPORT("void BeginTabBar(const string&in id, int flags = 0)", BeginTabBarAs);

		/*
		!CompatMeta
		for     UI
		with    void EndTabBar()
		as      void EndTabBar()
		being   function
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		Gecko_Exports_ImGui_MakeEndFn(TabBar);

		/*
		!CompatMeta
		for     UI
		with    bool BeginTabItem(const string&in label, int flags = UI::TabItemFlags::None)
		as      bool BeginTabItem(const string&in label, int flags)
		being   function
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		bool BeginTabItemAs_NoOpen(std::u8string& label, int flags)
		{
			Gecko_Exports_ImGui_GuardNonRender(false);
			return BeginTabItem((const char*)label.c_str(), nullptr, flags);
		}
		UI_EXPORT("bool BeginTabItem(const string&in label, int flags = 0)", BeginTabItemAs_NoOpen);

		/*
		!CompatMeta
		for     UI
		with    bool BeginTabItem(const string&in label, bool&out open, int flags = UI::TabItemFlags::None)
		as      bool BeginTabItem(const string&in label, bool&out open, int flags)
		being   function
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		bool BeginTabItemAs_Open(std::u8string& label, bool* open, int flags)
		{
			Gecko_Exports_ImGui_GuardNonRender(false);
			return BeginTabItem((const char*)label.c_str(), open, flags);
		}
		UI_EXPORT("bool BeginTabItem(const string&in label, bool&out open, int flags = 0)", BeginTabItemAs_Open);

		/*
		!CompatMeta
		for     UI
		with    void EndTabItem()
		as      void EndTabItem()
		being   function
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		Gecko_Exports_ImGui_MakeEndFn(TabItem);

		/*
		!CompatMeta
		for     UI
		with    bool BeginTable(const string&in id, int columns, int flags = UI::TableFlags::None, const vec2&in outer_size = vec2(), float inner_width = 0.0f)
		as      bool BeginTable(const string&in id, int columns, int flags)
		being   function
		declare partial
		reason  vec2 is not implemented yet
		since   v0.1.1
		!CompatMeta
		*/
		bool BeginTableAs(std::u8string& id, int columns, int flags)
		{
			Gecko_Exports_ImGui_GuardNonRender(false);
			return BeginTable((const char*)id.c_str(), columns, flags);
		}
		UI_EXPORT("bool BeginTable(const string&in id, int columns, int flags)", BeginTableAs);

		/*
		!CompatMeta
		for     UI
		with    void EndTable()
		as      void EndTable()
		being   function
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		Gecko_Exports_ImGui_MakeEndFn(Table);

		/*
		!CompatMeta
		for     UI
		with    void BeginTooltip()
		as      void BeginTooltip()
		being   function
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		Gecko_Exports_ImGui_MakeTrivialFn(BeginTooltip);

		/*
		!CompatMeta
		for     UI
		with    bool BeginItemTooltip()
		as      bool BeginItemTooltip()
		being   function
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		Gecko_Exports_ImGui_MakeTrivialReturnFn(BeginItemTooltip, bool);

		/*
		!CompatMeta
		for     UI
		with    void EndTooltip()
		as      void EndTooltip()
		being   function
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		Gecko_Exports_ImGui_MakeEndFn(Tooltip);
	}

	// General widgets
	namespace
	{
		/*
		!CompatMeta
		for     UI
		with    void Text(const string&in text, int length = -1)
		as      void Text(const string&in text, int length = -1)
		being   function
		declare full
		since   v0.1.0
		!CompatMeta
		*/
		void TextAs(std::u8string& s, int length)
		{
			Gecko_Exports_ImGui_GuardNonRender();

			if (length != -1)
			{
				std::u8string_view vu(s);
				vu = vu.substr(0, length);
				Text("%.*s", vu.length(), (const char*)vu.data());
			}
			else
			{
				Text((const char*)s.c_str());
			}
		}
		UI_EXPORT("void Text(const string&in text, int length = -1)", TextAs);

		/*
		!CompatMeta
		for     UI
		with    void TextWrapped(const string&in text)
		as      void TextWrapped(const string&in text)
		being   function
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		void TextWrappedAs(std::u8string& s)
		{
			Gecko_Exports_ImGui_GuardNonRender();

			TextWrapped((const char*)s.c_str());
		}
		UI_EXPORT("void TextWrapped(const string&in text)", TextWrappedAs);

		/*
		!CompatMeta
		for     UI
		with    void TextDisabled(const string&in text)
		as      void TextDisabled(const string&in text)
		being   function
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		void TextDisabledAs(std::u8string& s)
		{
			Gecko_Exports_ImGui_GuardNonRender();

			TextDisabled((const char*)s.c_str());
		}
		UI_EXPORT("void TextDisabled(const string&in text)", TextDisabledAs);

		/*
		!CompatMeta
		for     UI
		with    bool TextLink(const string&in label)
		as      bool TextLink(const string&in label)
		being   function
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		bool TextLinkAs(std::u8string& s)
		{
			Gecko_Exports_ImGui_GuardNonRender(false);

			return TextLink((const char*)s.c_str());
		}
		UI_EXPORT("bool TextLink(const string&in label)", TextLinkAs);

		/*
		!CompatMeta
		for     UI
		with    void TextLinkOpenURL(const string&in label, const string&in url)
		as      void TextLinkOpenURL(const string&in label, const string&in url)
		being   function
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		void TextLinkOpenURLAs(std::u8string& l, std::u8string& t)
		{
			Gecko_Exports_ImGui_GuardNonRender();

			TextLinkOpenURL((const char*)l.c_str(), (const char*)t.c_str());
		}
		UI_EXPORT("void TextLinkOpenURL(const string&in label, const string&in text)", TextLinkOpenURLAs);

		/*
		!CompatMeta
		for     UI
		with    void LabelText(const string&in label, const string&in text)
		as      void LabelText(const string&in label, const string&in text)
		being   function
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		void LabelTextAs(std::u8string& l, std::u8string& t)
		{
			Gecko_Exports_ImGui_GuardNonRender();

			LabelText((const char*)l.c_str(), (const char*)t.c_str());
		}
		UI_EXPORT("void LabelText(const string&in label, const string&in text)", LabelTextAs);

		/*
		!CompatMeta
		for     UI
		with    bool Button(const string&in label, const vec2&in size = vec2())
		as      bool Button(const string&in label)
		being   function
		declare partial
		reason  vec2 is not implemented yet
		since   v0.1.1
		!CompatMeta
		*/
		bool ButtonAs(std::u8string& s)
		{
			Gecko_Exports_ImGui_GuardNonRender(false);
			return Button((const char*)s.c_str());
		}
		UI_EXPORT("bool Button(const string&in text)", ButtonAs);

		/*
		!CompatMeta
		for     UI
		with    bool Checkbox(const string&in label, bool value)
		as      bool Checkbox(const string&in label, bool value)
		being   function
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		bool CheckboxAs(std::u8string& s, bool value)
		{
			Gecko_Exports_ImGui_GuardNonRender(false);
			Checkbox((const char*)s.c_str(), &value);
			return value;
		}
		UI_EXPORT("bool Checkbox(const string&in label, bool value)", CheckboxAs);


		/*
		!CompatMeta
		for     UI
		with    void Image(Texture@ texture)
		as      void Image(Texture@ texture)
		being   function
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		void ImageAs(Texture* Tex)
		{
			Gecko_Exports_ImGui_GuardNonRender();
			if (!Tex) return;
			Image(Tex->Get(), Tex->GetSize());
		}
		UI_EXPORT("void Image(Texture@+ texture)", ImageAs);
	}

	// Flags and state
	namespace 
	{
		/*
		!CompatMeta
		for     UI
		with    bool IsItemHovered(int flags = UI::HoveredFlags::None)
		as      bool IsItemHovered(int flags = 0)
		being   function
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		bool IsItemHoveredAs(int flags)
		{
			Gecko_Exports_ImGui_GuardNonRender(false);
			return IsItemHovered(flags);
		}
		UI_EXPORT("bool IsItemHovered(int flags = 0)", IsItemHoveredAs);

		/*
		!CompatMeta
		for     UI
		with    bool IsItemActive()
		as      bool IsItemActive()
		being   function
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		Gecko_Exports_ImGui_MakeTrivialReturnFn(IsItemActive, bool);

		/*
		!CompatMeta
		for     UI
		with    bool IsItemFocused()
		as      bool IsItemFocused()
		being   function
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		Gecko_Exports_ImGui_MakeTrivialReturnFn(IsItemFocused, bool);

		/*
		!CompatMeta
		for     UI
		with    void PushID(const string&in id)
		as      void PushID(const string&in id)
		being   function
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		void PushIDAs(std::u8string& id)
		{
			Gecko_Exports_ImGui_GuardNonRender();
			PushID((const char*)id.c_str());
		}
		UI_EXPORT("void PushID(const string&in id)", PushIDAs);

		/*
		!CompatMeta
		for     UI
		with    void PopID()
		as      void PopID()
		being   function
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		Gecko_Exports_ImGui_MakeTrivialFn(PopID);

		/*
		!CompatMeta
		for     UI
		with    void PushItemWidth(float item_width)
		as      void PushItemWidth(float item_width)
		being   function
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		void PushItemWidthAs(float w)
		{
			Gecko_Exports_ImGui_GuardNonRender();
			PushItemWidth(w);
		}
		UI_EXPORT("void PushItemWidth(float item_width)", PushItemWidthAs);

		/*
		!CompatMeta
		for     UI
		with    void PopItemWidth()
		as      void PopItemWidth()
		being   function
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		Gecko_Exports_ImGui_MakeTrivialFn(PopItemWidth);

		/*
		!CompatMeta
		for     UI
		with    void SameLine(float offset_from_start_x = 0.0f, float spacing = -1.0f)
		as      void SameLine(float offset_from_start_x = 0.0f, float spacing = -1.0f)
		being   function
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		void SameLineAs(float ox, float sp)
		{
			Gecko_Exports_ImGui_GuardNonRender();
			SameLine(ox, sp);
		}
		UI_EXPORT("void SameLine(float offset_from_start_x = 0.0f, float spacing = -1.0f)", SameLineAs);

		/*
		!CompatMeta
		for     UI
		with    void Separator()
		with    void Separator(int flags, float thickness = 1.0f)
		as      void Separator()
		being   function
		declare partial
		reason  flags and thickness variant is not implemented
		since   v0.1.1
		!CompatMeta
		*/
		Gecko_Exports_ImGui_MakeTrivialFn(Separator);
	}

	void Registrar(asIScriptEngine* Engine)
	{
		Engine->SetDefaultNamespace("UI");
		Engine->RegisterObjectType("Texture", sizeof(Texture), asOBJ_REF);

		/*
		!CompatMeta
		for     UI
		with    void $beh5()
		as      void $beh5()
		being   behavior
		of      Texture
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		Engine->RegisterObjectBehaviour("Texture", asBEHAVE_ADDREF, "void f()", asMETHOD(Texture, AddRef), asCALL_THISCALL);

		/*
		!CompatMeta
		for     UI
		with    void $beh6()
		as      void $beh6()
		being   behavior
		of      Texture
		declare full
		since   v0.1.1
		!CompatMeta
		*/
		Engine->RegisterObjectBehaviour("Texture", asBEHAVE_RELEASE, "void f()", asMETHOD(Texture, RemoveRef), asCALL_THISCALL);
		Engine->SetDefaultNamespace("");

		RegisterAll(Engine, s_Exports);
	}
}
