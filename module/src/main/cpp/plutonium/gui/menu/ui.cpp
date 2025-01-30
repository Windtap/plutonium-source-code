#include "ui.hpp"

#include "widgets.hpp"
#include "fonts.hpp"

#include "imgui/misc/freetype/imgui_freetype.h"

#include "bytes.hpp"
#include "font_awesome.hpp"
#include "notifications.hpp"
#include "log.hpp"

using namespace ImGui;

template <typename T>
inline T _clamp(const T& n, const T& lower, const T& upper) {
	return std::max(lower, std::min(n, upper));
}
inline float lerp(float a, float b, float f) {
	return _clamp<float>(a + f * (b - a), a > b ? b : a, a > b ? a : b);
}

void custom_interface_t::initialize() {
	auto& style = GetStyle();
	auto& col = GetStyle().Colors;

	style.ScrollbarSize = 6;
	style.ScrollbarRounding = 3;
	style.FrameRounding = 4;
	style.WindowRounding = 6;
	style.WindowBorderSize = 0;
	style.ItemSpacing = { 10, 10 };
	style.WindowMinSize = { 2, 2 };

	col[ImGuiCol_Border] = ImColor(28, 28, 28).Value;
	col[ImGuiCol_WindowBg] = ImColor(16, 16, 16).Value;
	col[ImGuiCol_ChildBg] = { 0, 0, 0, 0 };
	col[ImGuiCol_ScrollbarBg] = ImColor(10, 10, 10).Value;

	c_scale->current_scale = 2.f;

	style.ScaleAllSizes(c_scale->current_scale);
}

void custom_interface_t::initialize_fonts() {
	auto& io = GetIO();

	ImFontConfig cfg;
	//cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags::ImGuiFreeTypeBuilderFlags_ForceAutoHint | ImGuiFreeTypeBuilderFlags::ImGuiFreeTypeBuilderFlags_Monochrome;
	//cfg.RasterizerFlags = ImGuiFreeType::RasterizerFlags::MonoHinting | ImGuiFreeType::RasterizerFlags::Monochrome;

	this->fonts.regular = io.Fonts->AddFontFromMemoryTTF(montserrat_med, sizeof montserrat_med, 14.f * c_scale->current_scale, &cfg, io.Fonts->GetGlyphRangesCyrillic());

	ImFontConfig icons_config;
	icons_config.MergeMode = true;
	icons_config.SizePixels = 14.f * c_scale->current_scale;
	static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0x0 };
	ImFontConfig icons_config2;
	icons_config2.SizePixels = 26 * c_scale->current_scale;
	//icons_config.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags::ImGuiFreeTypeBuilderFlags_ForceAutoHint | ImGuiFreeTypeBuilderFlags::ImGuiFreeTypeBuilderFlags_Monochrome;

	this->fonts.icons = io.Fonts->AddFontFromMemoryTTF(font_awesome_binary, sizeof font_awesome_binary, 9 * c_scale->current_scale, &icons_config, icon_ranges);

	this->fonts.bold = io.Fonts->AddFontFromMemoryTTF(montserrat_med, sizeof montserrat_med, 14.f * c_scale->current_scale, &cfg, io.Fonts->GetGlyphRangesCyrillic());
	this->fonts.big_and_bold = io.Fonts->AddFontFromMemoryTTF(montserrat_med, sizeof montserrat_med, 20 * c_scale->current_scale, &cfg, io.Fonts->GetGlyphRangesCyrillic());
	this->fonts.logo = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(plutonium_logo_data, sizeof plutonium_logo_data, 40.f * c_scale->current_scale, nullptr, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

	this->fonts.gondon = io.Fonts->AddFontFromMemoryTTF(font_awesome_binary, sizeof font_awesome_binary, 16 * c_scale->current_scale, &icons_config2, icon_ranges);

	this->tabs = {
		{("Combat"), ICON_FA_MOUNTAIN, 0},
		{("Visuals"), ICON_FA_EYE_LOW_VISION, 2},
		{("Exploits"), ICON_FA_CIRCLE_USER, 3},
		{("Inventory"), ICON_FA_BOX},
		{("Misc"), ICON_FA_GEAR},
        {("Cloud"), ICON_FA_CLOUD},
	};

	this->subtabs = {
		{("AimBot"), 0},
		{("KnifeBot"), 0},
		{("Player"), 1},
        {("Player"), 2}
	};

	c_widgets->arrow_elem.font = this->fonts.icons;
	c_widgets->arrow_elem.glyph = ICON_FA_ANGLE_DOWN;
	c_widgets->arrow_elem.glyph_up = ICON_FA_ANGLE_UP;
}

void custom_interface_t::render(void *_vars) {
	ShowDemoWindow();
    cvars *vars = (cvars *) _vars;

    const char *bones_name[] = {
            "Head",
            "Neck",
            "Stomach",
            "Hands",
            "Legs",
            "Foots"
    };


#define GETPOINTER(type, structn, name) ((type *) (((uintptr_t) vars + offsetof(cvars, structn.name))))

    bool *enable_esp = (bool *) ((uintptr_t) vars + offsetof(cvars, bools.enable_esp));

	// --- //

	logs::render(ImGui::GetIO().DisplaySize, ImGui::GetBackgroundDrawList(), 25);

	ImGui::RenderNotifications();

	PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });

	static ImVec2 main_window_pos{};

	static animation child_bg{};

	child_bg.update(1);

	SetNextWindowSize({ c_scale->get(620), c_scale->get(370) - c_scale->get(45) });

	Begin("##t.me/imguiseller", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus); {
		GetWindowDrawList()->AddRectFilled(GetWindowPos(), GetWindowPos() + GetWindowSize(), ImColor(15, 15, 15), GetStyle().WindowRounding, ImDrawFlags_RoundCornersBottom);

		main_window_pos = GetWindowPos();

		auto& col = GetStyle().Colors;
		col[ImGuiCol_ScrollbarGrabActive] = c_utils->get_accent_imv4(GetStyle().Alpha, 0.8f);
		col[ImGuiCol_ScrollbarGrabHovered] = c_utils->get_accent_imv4(GetStyle().Alpha, 0.8f);
		col[ImGuiCol_ScrollbarGrab] = c_utils->get_accent_imv4(GetStyle().Alpha, 0.8f);
		//col[ImGuiCol_ScrollbarBg] = c_utils->get_accent_imv4(GetStyle().Alpha, 0.05f);

		// --- //

		/* tabs --> */ {
			BeginChild("##tabs", { c_scale->get(150), GetWindowSize().y }, false);

			static animation_vec2 background{};

			SetCursorPosY(GetCursorPosY() + c_scale->get(10));

			const ImVec2& size = { GetContentRegionAvail().x, c_scale->get(30) };

			//GetWindowDrawList()->AddShadowRect(GetWindowPos() + background.value + ImVec2(c_scale->get(9), 0), GetWindowPos() + background.value + size - ImVec2(c_scale->get(9), 0), static_cast<ImColor>(c_utils->process_alpha(ImColor(0, 0, 0).Value, GetStyle().Alpha)), c_scale->get(10), { 0, 0 }, 0, c_scale->get(4));
			GetWindowDrawList()->AddRectFilled(GetWindowPos() + background.value + ImVec2(c_scale->get(9), 0), GetWindowPos() + background.value + size - ImVec2(c_scale->get(9), 0), static_cast<ImColor>(c_utils->process_alpha(ImColor(20, 20, 20).Value, GetStyle().Alpha)), GetStyle().WindowRounding);
			//GetWindowDrawList()->AddRect(GetWindowPos() + background.value + ImVec2(c_scale->get(9), 0), GetWindowPos() + background.value + size - ImVec2(c_scale->get(9), 0), static_cast<ImColor>(c_utils->process_alpha(ImColor(24, 24, 24).Value, GetStyle().Alpha)), GetStyle().WindowRounding, 0, c_scale->get(1));

			PushStyleVar(ImGuiStyleVar_ItemSpacing, { c_scale->get(10), 0 });

			for (int it{}; it < this->tabs.size(); it++) {
				auto& tab = this->tabs[it];

				const ImVec2& current_cursor = GetCursorScreenPos();
				const ImVec2& current_cursor_window = GetCursorPos();

				if (InvisibleButton(tab.name, size)) {
					this->current_tab = it;
					this->current_subtab = tab.default_sub;
					child_bg.value = 0;
					//ImGui::Notification({ ImGuiToastType_None, 4000, "Example notification | Buy best menus at t.me/imguiseller" });
				}

				const ImRect btn_rect{
					current_cursor + ImVec2(c_scale->get(9), 0),
					current_cursor + size - ImVec2(c_scale->get(9), 0)
				};

				const auto& draw_list = GetWindowDrawList();

				const auto& icon_size = this->fonts.icons->CalcTextSizeA(c_scale->get(9), FLT_MAX, -1, tab.name);

				const auto& label_size = CalcTextSize(tab.name);

				tab.text.interpolate(ImColor(255, 255, 255).Value, ImColor(150, 150, 150).Value, (it == this->current_tab));

				tab._icon.interpolate(c_utils->get_accent_imv4(GetStyle().Alpha), ImColor(150, 150, 150).Value, (it == this->current_tab));

				const ImVec2 text_pos{
					btn_rect.Min.x + btn_rect.GetHeight() / 2 - label_size.y / 2,
					btn_rect.GetCenter().y - label_size.y / 2,
				};

				if (tab.icon)
					draw_list->AddText(text_pos - ImVec2(0, 1 /* THIS NIGGA IS FUCKER LMAO RETARDED OCORNUT */), static_cast<ImColor>(c_utils->process_alpha(tab._icon.value, GetStyle().Alpha)), tab.icon);

				draw_list->AddText(text_pos + ImVec2(c_scale->get(20), 0) - ImVec2(0, 1 /* THIS NIGGA IS FUCKER LMAO RETARDED OCORNUT */), static_cast<ImColor>(c_utils->process_alpha(tab.text.value, GetStyle().Alpha)), tab.name);

				if (this->current_tab == it)
					background.update(current_cursor_window);

			}

			PopStyleVar();

			EndChild();

			GetWindowDrawList()->AddRectFilled(GetWindowPos(), GetWindowPos() + ImVec2(c_scale->get(150), GetWindowSize().y), ImColor(16, 16, 16), GetStyle().WindowRounding, ImDrawFlags_RoundCornersBottomLeft);
			GetWindowDrawList()->AddLine(GetWindowPos() + ImVec2(c_scale->get(150), 0), GetWindowPos() + ImVec2(c_scale->get(150), GetWindowSize().y), ImColor(24, 24, 24), c_scale->get(1));

		}

		// --- //

		/* main --> */ {
			PushStyleVar(ImGuiStyleVar_Alpha, child_bg.value * GetStyle().Alpha);

			SetCursorPos({ c_scale->get(150) + c_scale->get(10), 0 + 1 + (c_scale->get(20) * (1.0f - child_bg.value)) });

			const float& width = GetContentRegionAvail().x / 2 - c_scale->get(10);

			if (this->current_tab != 999) {
				BeginChild("##container_1", { width, GetContentRegionAvail().y - c_scale->get(1) }, false);
				Spacing();
				static ImVec4 col, c_col;
				static bool test;
				static int slider;
				static bool test3;
				static const char* fov_types[] = { "Circle", "Rect" };
                static const char* aim_by[] = { "Max damage", "Distance to bone" };
				static bool* _ret[3] = { &test, &test3 };
				static bool depend_test;
				switch (this->current_tab)
                {
                    case 0:
                        c_widgets->toggle("Enable", GETPOINTER(bool, bools, enable_aimbot));
                        c_widgets->toggle("Fire Check", GETPOINTER(bool, bools, enable_firecheck));
                        c_widgets->toggle("Untouchable Check", GETPOINTER(bool, bools, enable_untouchable_check));
                        c_widgets->toggle("Silent", GETPOINTER(bool, bools, enable_silent));
                        c_widgets->multi_combo("Bones", GETPOINTER(bool *, bools, enable_bone), bones_name, IM_ARRAYSIZE(bones_name));
                        break;

                    case 1:
                        c_widgets->toggle("Enable", GETPOINTER(bool, bools, enable_esp), GETPOINTER(ImVec4, colors, box_color));
                        c_widgets->toggle("Box", GETPOINTER(bool, bools, enable_box));
                        c_widgets->toggle("Health Bar", GETPOINTER(bool, bools, enable_hpbar));
                        c_widgets->toggle("Name", GETPOINTER(bool, bools, enable_name));
                        c_widgets->toggle("Line", GETPOINTER(bool, bools, enable_line));
                        c_widgets->toggle("Skeleton", GETPOINTER(bool, bools, enable_skeleton), GETPOINTER(ImVec4, colors, skeleton_visible_color), GETPOINTER(ImVec4, colors, skeleton_invisible_color));
                        c_widgets->toggle("Hitboxes", GETPOINTER(bool, bools, enable_hitboxes), GETPOINTER(ImVec4, colors, skeleton_visible_color), GETPOINTER(ImVec4, colors, skeleton_invisible_color));
                        break;

                    case 2:
                        c_widgets->toggle("Invisible", GETPOINTER(bool, bools, enable_invisible_exploit));
                        break;
                }

				Spacing();
				EndChild();

				SameLine(0, c_scale->get(10));

				BeginChild("##container_2", { width, GetContentRegionAvail().y - c_scale->get(1) }, false);
				Spacing();

				switch (this->current_tab) {
				case 0:
                    c_widgets->combo("Fov Type", GETPOINTER(int, integers, fov_type), fov_types, IM_ARRAYSIZE(fov_types));
                    c_widgets->combo("Bone filter", GETPOINTER(int, integers, aiming_type), aim_by, IM_ARRAYSIZE(aim_by));
                    c_widgets->color_edit_4("Fov Color", GETPOINTER(float, colors, fov_color));
                    c_widgets->slider_float("Fov", GETPOINTER(float, floats, aimbot_fov), 0, 720, "%.1f");

                    if (*GETPOINTER(int, integers, fov_type) == 1)
                    {
                        c_widgets->slider_float("Fov Height", GETPOINTER(float, floats, aimbot_fovy), 0, 720, "%.1f");
                    }

                    if (!*GETPOINTER(bool, bools, enable_silent))
                    {
                        c_widgets->slider_float("Smooth", GETPOINTER(float, floats, aimbot_smooth), 0, 150, "%.1f");
                    }
					break;
				case 1:
                    c_widgets->slider_float("Thickness", GETPOINTER(float, floats, box_thickness), 1, 10, "%.1f");
					break;
				}

				Spacing();

				EndChild();
			}
			else
            {
				BeginChild("##container_settings", { GetContentRegionAvail().x - c_scale->get(10), GetContentRegionAvail().y - c_scale->get(1)}, false);
				Spacing();
				c_widgets->color_edit_4("Menu accent color", (float*)c_utils->get_accent_imv4_ptr(), ImGuiColorEditFlags_NoAlpha);
				EndChild();
			}

			PopStyleVar();
		}

		//GetWindowDrawList()->AddRect(GetWindowPos(), GetWindowPos() + GetWindowSize(), ImColor(24, 24, 24), GetStyle().WindowRounding, ImDrawFlags_RoundCornersBottom, c_scale->get(1));

		End();
	}

	SetNextWindowPos(main_window_pos - ImVec2(0, c_scale->get(70)));
	SetNextWindowSize({ c_scale->get(620), c_scale->get(70) });

	Begin("##heading", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove); {
		GetWindowDrawList()->AddRectFilled(GetWindowPos(), GetWindowPos() + GetWindowSize(), ImColor(15, 15, 15), GetStyle().WindowRounding, ImDrawFlags_RoundCornersTop);

		int heading_height = c_scale->get(70);
		const auto& base = GetWindowPos();

		const ImRect h_rect{
			base,
			base + ImVec2(GetWindowSize().x, heading_height)
		};

		auto draw_list = GetWindowDrawList();

		PushFont(this->fonts.logo);

		const char* heading_text = "A";
		const auto& h_s = CalcTextSize(heading_text);

		int vert_start_idx_t = draw_list->VtxBuffer.Size;

		draw_list->AddText({ h_rect.Min.x + (c_scale->get(150) / 2 - h_s.y / 2), h_rect.GetCenter().y - h_s.y / 2 }, c_utils->get_accent_imc(), heading_text);

		int vert_end_idx_t = draw_list->VtxBuffer.Size;

		ShadeVertsLinearColorGradientKeepAlpha(draw_list, vert_start_idx_t, vert_end_idx_t, h_rect.GetCenter() - h_s / 2, h_rect.GetCenter() + h_s / 2, c_utils->get_accent_imc(), ImColor(255, 255, 255));

		PopFont();

        draw_list->AddText({ h_rect.Min.x + (c_scale->get(150) / 2 + h_s.y / 2), h_rect.GetCenter().y + h_s.y / 3 }, ImColor(255, 0, 0), "beta");

		// --- //
		//GetWindowDrawList()->AddLine({ GetWindowPos().x + GetWindowSize().x - GetWindowSize().y, GetWindowPos().y}, {GetWindowPos().x + GetWindowSize().x - GetWindowSize().y, GetWindowPos().y + GetWindowSize().y - 1}, ImColor(28, 28, 28), c_scale->get(1));
		GetWindowDrawList()->AddLine({ GetWindowPos().x + c_scale->get(150), GetWindowPos().y }, { GetWindowPos().x + c_scale->get(150), GetWindowPos().y + GetWindowSize().y - 1 }, ImColor(28, 28, 28), c_scale->get(1));
		GetWindowDrawList()->AddLine({ GetWindowPos().x, GetWindowPos().y + GetWindowSize().y - 1 }, GetWindowPos() + GetWindowSize() - ImVec2(0, 1), ImColor(28, 28, 28), c_scale->get(1));

		SetCursorPos({ c_scale->get(150) + GetStyle().ItemSpacing.x + c_scale->get(16), 10 });

		/* subtabs --> */ {
			for (int it{}; it < subtabs.size(); it++) {
				if (subtabs[it].parent != current_tab) continue;

				PushStyleVar(ImGuiStyleVar_ItemSpacing, { c_scale->get(8), 0 });

				SetCursorPosY(c_scale->get(10));

				const auto& current_cursor = GetCursorScreenPos();
				const auto& btn_sz = ImVec2(CalcTextSize(subtabs[it].name).x + c_scale->get(6), c_scale->get(50));

				if (InvisibleButton(subtabs[it].name, btn_sz)) {
					current_subtab = it;
					child_bg.value = 0;
				}

				const ImRect btn_rect = {
					current_cursor,
					current_cursor + btn_sz
				};

				subtabs[it].text.interpolate(ImColor(255, 255, 255).Value, ImColor(150, 150, 150).Value, (it == this->current_subtab));

				subtabs[it]._icon.interpolate(c_utils->get_accent_imv4(GetStyle().Alpha), ImColor(150, 150, 150).Value, (it == this->current_subtab));

				GetWindowDrawList()->AddText(btn_rect.GetCenter() - CalcTextSize(subtabs[it].name) / 2, ImColor(subtabs[it].text.value), subtabs[it].name);
				GetWindowDrawList()->AddLine({ btn_rect.Min.x, btn_rect.GetCenter().y + CalcTextSize(subtabs[it].name).y / 2 + c_scale->get(2) }, { btn_rect.Max.x, btn_rect.GetCenter().y + CalcTextSize(subtabs[it].name).y / 2 + c_scale->get(2) }, ImColor(subtabs[it]._icon.value), c_scale->get(2));

				SameLine();

				PopStyleVar();
			}

			const ImVec2 btn_sz = { GetWindowSize().y, GetWindowSize().y };
			SetCursorPos({ GetWindowSize().x - btn_sz.x, 0 });
			const auto& current_cursor_gear = GetCursorScreenPos();
			//GOVNOCODER VO MNE $$$
			if (InvisibleButton(ICON_FA_GEAR, btn_sz)) {
				current_tab = 999; //settings tab
				child_bg.value = 0;
			}
			const ImRect btn_rect_gear = {
				current_cursor_gear,
				current_cursor_gear + btn_sz
			};
			const auto& ic_sz = fonts.gondon->CalcTextSizeA(16 * c_scale->current_scale, FLT_MAX, 0, ICON_FA_GEAR);
			GetWindowDrawList()->AddText(fonts.gondon, 16 * c_scale->current_scale, btn_rect_gear.GetCenter() - ic_sz / 2, ImColor(255, 255, 255, 100), ICON_FA_GEAR);
		}

		End();
	}

	PopStyleVar();
}