#include "internal.hpp"

using namespace ImGui;

bool color_button_internal(const char* desc_id, const ImVec4& col, ImGuiColorEditFlags flags, const ImVec2& size_arg)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiID id = window->GetID(desc_id);
    const float default_size = GetFrameHeight();
    const ImVec2 size(size_arg.x == 0.0f ? default_size : size_arg.x, size_arg.y == 0.0f ? default_size : size_arg.y);
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);

    ItemSize(bb, (size.y >= default_size) ? g.Style.FramePadding.y : 0.0f);

    if (!ItemAdd(bb, id))
        return false;

    bool hovered, held, pressed = ButtonBehavior(bb, id, &hovered, &held);

    ImVec4 col_rgb_without_alpha(col.x, col.y, col.z, 1.0f);

    float grid_step = ImMin(size.x, size.y) / c_scale->get(2.99f);
    float off = 0.0f;
    ImRect bb_inner = bb;
    off = -c_scale->get(0.75f);
    bb_inner.Expand(off);
    ImVec4 col_rgb = col;
    ImVec4 col_source = col_rgb_without_alpha;
    if ((flags & ImGuiColorEditFlags_AlphaPreviewHalf) && col_rgb.w < 1.0f)
    {
        float mid_x = IM_ROUND((bb_inner.Min.x + bb_inner.Max.x) * 0.5f);
        RenderColorRectWithAlphaCheckerboard(window->DrawList, ImVec2(bb_inner.Min.x + grid_step, bb_inner.Min.y), bb_inner.Max, GetColorU32(col_rgb), grid_step, ImVec2(-grid_step + off, off), c_scale->get(8), ImDrawFlags_RoundCornersRight);
        window->DrawList->AddRectFilled(bb_inner.Min, ImVec2(mid_x, bb_inner.Max.y), GetColorU32(col_rgb_without_alpha), 8, ImDrawFlags_RoundCornersLeft);
    }
    else
    {
        // Because GetColorU32() multiplies by the global style Alpha and we don't want to display a checkerboard if the source code had no alpha
        ImVec4 col_source = (flags & ImGuiColorEditFlags_AlphaPreview) ? col_rgb : col_rgb_without_alpha;
        if (col_source.w < 1.0f)
            RenderColorRectWithAlphaCheckerboard(window->DrawList, bb_inner.Min, bb_inner.Max, GetColorU32(col_source), grid_step, ImVec2(off, off), c_scale->get(8));
        else
            window->DrawList->AddRectFilled(bb_inner.Min, bb_inner.Max, GetColorU32(col_source), c_scale->get(8));
    }
    //RenderColorRectWithAlphaCheckerboard(window->DrawList, bb_inner.Min, bb_inner.Max, GetColorU32(col_source), grid_step, ImVec2(off, off), 8);

    return pressed;
}

bool color_picker_internal(const char* label, float col[4], ImGuiColorEditFlags flags, const float* ref_col)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImDrawList* draw_list = window->DrawList;
    ImGuiStyle& style = g.Style;
    ImGuiIO& io = g.IO;
    ImGuiID id = GetID(label);

    static struct data {
        animation_vec2 first;
        animation second;
        animation_vec2 thirth;
        animation_vec2 fourth;
        bool set_defaults = true;
    };

    static std::map<ImGuiID, data> anim;
    data& state = anim[id];

    const float width = CalcItemWidth();
    g.NextItemData.ClearFlags();

    PushID(label);
    const bool set_current_color_edit_id = (g.ColorEditCurrentID == 0);
    if (set_current_color_edit_id)
        g.ColorEditCurrentID = window->IDStack.back();
    BeginGroup();

    if (!(flags & ImGuiColorEditFlags_PickerMask_))
        flags |= ((g.ColorEditOptions & ImGuiColorEditFlags_PickerMask_) ? g.ColorEditOptions : ImGuiColorEditFlags_DefaultOptions_) & ImGuiColorEditFlags_PickerMask_;
    if (!(flags & ImGuiColorEditFlags_InputMask_))
        flags |= ((g.ColorEditOptions & ImGuiColorEditFlags_InputMask_) ? g.ColorEditOptions : ImGuiColorEditFlags_DefaultOptions_) & ImGuiColorEditFlags_InputMask_;
    IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags_PickerMask_)); // Check that only 1 is selected
    IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags_InputMask_));  // Check that only 1 is selected
    if (!(flags & ImGuiColorEditFlags_NoOptions))
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags_AlphaBar);

    // Setup
    bool alpha_bar = true;
    ImVec2 picker_pos = window->DC.CursorPos;
    float square_sz = GetFrameHeight();
    float bars_width = square_sz / 2; // Arbitrary smallish width of Hue/Alpha picking bars
    float sv_picker_size = ImMax(bars_width * 1, width - (alpha_bar ? 2 : 1) * (bars_width + style.ItemInnerSpacing.x)); // Saturation/Value picking box
    float bar0_pos_x = picker_pos.x + sv_picker_size + style.ItemInnerSpacing.x;
    float bar1_pos_x = bar0_pos_x + bars_width + style.ItemInnerSpacing.x;
    float bars_triangles_half_sz = IM_TRUNC(bars_width * 0.20f);

    float backup_initial_col[4];
    memcpy(backup_initial_col, col, 4 * sizeof(float));

    float wheel_thickness = sv_picker_size * 0.08f;
    float wheel_r_outer = sv_picker_size * 0.50f;
    float wheel_r_inner = wheel_r_outer - wheel_thickness;
    ImVec2 wheel_center(picker_pos.x + (sv_picker_size + bars_width) * 0.5f, picker_pos.y + sv_picker_size * 0.5f);

    // Note: the triangle is displayed rotated with triangle_pa pointing to Hue, but most coordinates stays unrotated for logic.
    float triangle_r = wheel_r_inner - (int)(sv_picker_size * 0.027f);
    ImVec2 triangle_pa = ImVec2(triangle_r, 0.0f); // Hue point.
    ImVec2 triangle_pb = ImVec2(triangle_r * -0.5f, triangle_r * -0.866025f); // Black point.
    ImVec2 triangle_pc = ImVec2(triangle_r * -0.5f, triangle_r * +0.866025f); // White point.

    float H = col[0], S = col[1], V = col[2];
    float R = col[0], G = col[1], B = col[2];

    ColorConvertRGBtoHSV(R, G, B, H, S, V);
    ColorEditRestoreHS(col, &H, &S, &V);

    bool value_changed = false, value_changed_h = false, value_changed_sv = false;

    PushItemFlag(ImGuiItemFlags_NoNav, true);

    // hue bar
    InvisibleButton("sv", ImVec2(sv_picker_size, sv_picker_size));
    if (IsItemActive())
    {
        S = ImSaturate((io.MousePos.x - picker_pos.x) / (sv_picker_size - 1));
        V = 1.0f - ImSaturate((io.MousePos.y - picker_pos.y) / (sv_picker_size - 1));
        ColorEditRestoreH(col, &H); // Greatly reduces hue jitter and reset to 0 when hue == 255 and color is rapidly modified using SV square.
        value_changed = value_changed_sv = true;
    }
    if (!(flags & ImGuiColorEditFlags_NoOptions))
        OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);

    SetCursorScreenPos(ImVec2(bar0_pos_x, picker_pos.y));
    InvisibleButton("hue", ImVec2(bars_width, sv_picker_size));
    if (IsItemActive())
    {
        H = ImSaturate((io.MousePos.y - picker_pos.y) / (sv_picker_size - 1));
        value_changed = value_changed_h = true;
    }

    //alpha bar
    SetCursorScreenPos(ImVec2(bar1_pos_x, picker_pos.y));
    InvisibleButton("alpha", ImVec2(bars_width, sv_picker_size));
    if (IsItemActive())
    {
        col[3] = 1.0f - ImSaturate((io.MousePos.y - picker_pos.y) / (sv_picker_size - 1));
        value_changed = true;
    }
    PopItemFlag(); // ImGuiItemFlags_NoNav

    // Convert back color to RGB
    if (value_changed_h || value_changed_sv)
    {
        ColorConvertHSVtoRGB(H, S, V, col[0], col[1], col[2]);
        g.ColorEditSavedHue = H;
        g.ColorEditSavedSat = S;
        g.ColorEditSavedID = g.ColorEditCurrentID;
        g.ColorEditSavedColor = ColorConvertFloat4ToU32(ImVec4(col[0], col[1], col[2], 0));
    }

    if (value_changed)
    {
        R = col[0];
        G = col[1];
        B = col[2];
        ColorConvertRGBtoHSV(R, G, B, H, S, V);
        ColorEditRestoreHS(col, &H, &S, &V);   // Fix local Hue as display below will use it immediately.
    }

    const int style_alpha8 = IM_F32_TO_INT8_SAT(style.Alpha);
    const ImU32 col_black = IM_COL32(0, 0, 0, style_alpha8);
    const ImU32 col_white = IM_COL32(255, 255, 255, style_alpha8);
    const ImU32 col_midgrey = IM_COL32(128, 128, 128, style_alpha8);
    const ImU32 col_hues[6 + 1] = { IM_COL32(255,0,0,style_alpha8), IM_COL32(255,255,0,style_alpha8), IM_COL32(0,255,0,style_alpha8), IM_COL32(0,255,255,style_alpha8), IM_COL32(0,0,255,style_alpha8), IM_COL32(255,0,255,style_alpha8), IM_COL32(255,0,0,style_alpha8) };

    ImVec4 hue_color_f(1, 1, 1, style.Alpha); ColorConvertHSVtoRGB(H, 1, 1, hue_color_f.x, hue_color_f.y, hue_color_f.z);
    ImU32 hue_color32 = ColorConvertFloat4ToU32(hue_color_f);
    ImU32 user_col32_striped_of_alpha = ColorConvertFloat4ToU32(ImVec4(R, G, B, style.Alpha)); // Important: this is still including the main rendering/style alpha!!

    ImVec2 sv_cursor_pos;
   
    // hue bar
    draw_list->AddRectFilledMultiColorRounded(picker_pos, picker_pos + ImVec2(sv_picker_size, sv_picker_size), col_white, hue_color32, hue_color32, col_white, 3);
    draw_list->AddRectFilledMultiColorRounded(picker_pos, picker_pos + ImVec2(sv_picker_size, sv_picker_size), 0, 0, col_black, col_black, 3);
    RenderFrameBorder(picker_pos, picker_pos + ImVec2(sv_picker_size, sv_picker_size), 0.0f);
    sv_cursor_pos.x = ImClamp(IM_ROUND(picker_pos.x + ImSaturate(S) * sv_picker_size), picker_pos.x + 2, picker_pos.x + sv_picker_size - 2); // Sneakily prevent the circle to stick out too much
    sv_cursor_pos.y = ImClamp(IM_ROUND(picker_pos.y + ImSaturate(1 - V) * sv_picker_size), picker_pos.y + 2, picker_pos.y + sv_picker_size - 2);

    // Render Hue Bar
    for (int i = 0; i < 6; ++i) {
        if (i == 0)
            draw_list->AddRectFilledMultiColorRounded(ImVec2(bar0_pos_x, picker_pos.y + i * (sv_picker_size / 6)), ImVec2(bar0_pos_x + bars_width, picker_pos.y + (i + 1) * (sv_picker_size / 6)), col_hues[i], col_hues[i], col_hues[i + 1], col_hues[i + 1], 3, ImDrawFlags_RoundCornersTop);
        else if (i != 0 && i != 5)
            draw_list->AddRectFilledMultiColorRounded(ImVec2(bar0_pos_x, picker_pos.y + i * (sv_picker_size / 6)), ImVec2(bar0_pos_x + bars_width, picker_pos.y + (i + 1) * (sv_picker_size / 6)), col_hues[i], col_hues[i], col_hues[i + 1], col_hues[i + 1]);
        else if (i >= 5)
            draw_list->AddRectFilledMultiColorRounded(ImVec2(bar0_pos_x, picker_pos.y + i * (sv_picker_size / 6)), ImVec2(bar0_pos_x + bars_width, picker_pos.y + (i + 1) * (sv_picker_size / 6)), col_hues[i], col_hues[i], col_hues[i + 1], col_hues[i + 1], 3, ImDrawFlags_RoundCornersBottom);
    }
    float bar0_line_y = IM_ROUND(picker_pos.y + H * sv_picker_size);
    RenderFrameBorder(ImVec2(bar0_pos_x, picker_pos.y), ImVec2(bar0_pos_x + bars_width, picker_pos.y + sv_picker_size), 0.0f);

    if (state.set_defaults)
        state.thirth.value = ImVec2(bar0_pos_x - 1, bar0_line_y);

    state.thirth.update(ImVec2(bar0_pos_x - 1, bar0_line_y));

    RenderArrowsForVerticalBar(draw_list, { bar0_pos_x, state.thirth.value.y }, ImVec2(bars_triangles_half_sz + 1, bars_triangles_half_sz), bars_width, style.Alpha);

    // Render cursor/preview circle (clamp S/V within 0..1 range because floating points colors may lead HSV values to be out of range)
    float sv_cursor_rad = value_changed_sv ? wheel_thickness * 0.45f : wheel_thickness * 0.35f;
    int sv_cursor_segments = draw_list->_CalcCircleAutoSegmentCount(sv_cursor_rad); // Lock segment count so the +1 one matches others.

    if (state.set_defaults)
        state.first.value = sv_cursor_pos;

    state.first.update(sv_cursor_pos);
    state.second.update(sv_cursor_rad);

    draw_list->AddCircle(state.first.value, state.second.value, user_col32_striped_of_alpha, sv_cursor_segments);
    draw_list->AddCircle(state.first.value, state.second.value + 1, col_midgrey, sv_cursor_segments);
    draw_list->AddCircle(state.first.value, state.second.value, col_white, sv_cursor_segments);

    //alpha bar
    float alpha = ImSaturate(col[3]);
    ImRect bar1_bb(bar1_pos_x, picker_pos.y, bar1_pos_x + bars_width, picker_pos.y + sv_picker_size);
    RenderColorRectWithAlphaCheckerboard(draw_list, bar1_bb.Min, bar1_bb.Max, 0, bar1_bb.GetWidth() / 2.0f, ImVec2(0.0f, 0.0f), 3);
    draw_list->AddRectFilledMultiColorRounded(bar1_bb.Min, bar1_bb.Max, user_col32_striped_of_alpha, user_col32_striped_of_alpha, user_col32_striped_of_alpha & ~IM_COL32_A_MASK, user_col32_striped_of_alpha & ~IM_COL32_A_MASK, 3);
    float bar1_line_y = IM_ROUND(picker_pos.y + (1.0f - alpha) * sv_picker_size);
    RenderFrameBorder(bar1_bb.Min, bar1_bb.Max, 3);

    if (state.set_defaults)
        state.fourth.value = ImVec2(bar1_pos_x - 1, bar1_line_y);

    state.fourth.update(ImVec2(bar1_pos_x - 1, bar1_line_y));

    RenderArrowsForVerticalBar(draw_list, { bar1_pos_x, state.fourth.value.y }, ImVec2(bars_triangles_half_sz + 1, bars_triangles_half_sz), bars_width, style.Alpha);

    EndGroup();

    if (value_changed && memcmp(backup_initial_col, col, 4 * sizeof(float)) == 0)
        value_changed = false;
    if (value_changed && g.LastItemData.ID != 0) // In case of ID collision, the second EndGroup() won't catch g.ActiveId
        MarkItemEdited(g.LastItemData.ID);

    if (set_current_color_edit_id)
        g.ColorEditCurrentID = 0;
    PopID();

    if (state.set_defaults)
        state.set_defaults = false;

    return value_changed;
}

bool call_colorpicker_internal(const char* label, float col[4], ImGuiColorEditFlags flags);

bool custom_elements_t::toggle(const char* label, bool* v, ImVec4* prov_color, ImVec4* prov_color2)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    const float square_sz = label_size.y;
    const ImVec2 pos = window->DC.CursorPos;
    const ImRect total_bb(
            pos,
            pos + ImVec2(calculate_width(), label_size.y + style.FramePadding.y * 6.0f)
    );

    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id))
    {
        IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
        return false;
    }

    struct cb_data {
        animation anim, hov{};
        animation_vec4 col, col2{};
        animation_vec4 outline, shadow, circle{};
        animation_vec4 text{};
    };

    static std::map < ImGuiID, cb_data > anim2;
    auto cb_anim = anim2.find(id);
    if (cb_anim == anim2.end())
    {
        anim2.insert({ id, {} });
        cb_anim = anim2.find(id);
    }

    if (*v)
        cb_anim->second.anim.update(1);
    else
        cb_anim->second.anim.update(0);

    const ImVec2& size = { square_sz * 2, square_sz };

    float size_off{};
    if (prov_color)
        size_off = 12 + GetFrameHeight() - 4;

    if (prov_color2)
    {
        size_off = 52 + GetFrameHeight() - 4;
    }

    const ImRect check_bb{
            ImVec2(total_bb.Max.x - size.x - style.FramePadding.x * 2 + c_scale->get(3) - size_off, total_bb.Min.y + total_bb.GetHeight() / 2 - size.y / 2),
            ImVec2(total_bb.Max.x - style.FramePadding.x * 2 + c_scale->get(3) - size_off, total_bb.Min.y + total_bb.GetHeight() / 2 + size.y / 2),
    };

    const ImRect touch_bb{
            total_bb.Min, check_bb.Max
    };

    bool hovered, held;
    bool pressed = ButtonBehavior(touch_bb, id, &hovered, &held);
    if (pressed)
    {
        *v = !(*v);
        MarkItemEdited(id);
    }

    if (hovered)
        cb_anim->second.hov.update(1);
    else
        cb_anim->second.hov.update(0);

    cb_anim->second.shadow.interpolate(c_utils->get_accent_imc(), ImColor(32, 32, 32).Value, * v);
    cb_anim->second.circle.interpolate(c_utils->get_accent_imc(), ImColor(32, 32, 32).Value, *v);

    cb_anim->second.col.interpolate(c_utils->get_accent_imc().Value, ImColor(24, 24, 24).Value, *v);

    cb_anim->second.col2.interpolate(c_utils->get_accent_imc(style.Alpha, 0.5f).Value, ImColor(24, 24, 24).Value, *v);

    window->DrawList->AddRectFilled(total_bb.Min, total_bb.Max, static_cast<ImColor>(c_utils->process_alpha(ImColor(18, 18, 18), style.Alpha)), style.FrameRounding);

    // toggle //

    //window->DrawList->Flags &= ~(ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines);

    window->DrawList->AddRectFilled(check_bb.Min + ImVec2(c_scale->get(3), c_scale->get(2)), check_bb.Max - ImVec2(c_scale->get(3), c_scale->get(2)), static_cast<ImColor>(c_utils->process_alpha(cb_anim->second.col2.value, style.Alpha)), c_scale->get(20));

    // window->DrawList->AddRect(check_bb.Min + ImVec2(3, 2), check_bb.Max - ImVec2(3, 2), static_cast<ImColor>(c_utils->process_alpha(cb_anim->second.col.value, style.Alpha)), 20);

    window->DrawList->AddShadowCircle(check_bb.Min + ImVec2(c_scale->get(2), 0) + ImVec2(check_bb.GetHeight() / 2 + (check_bb.GetWidth() / 2 - c_scale->get(4)) * cb_anim->second.anim.value, check_bb.GetHeight() / 2), (check_bb.GetHeight() / 2.2f), static_cast<ImColor>(c_utils->process_alpha(cb_anim->second.shadow.value, cb_anim->second.hov.value * style.Alpha)), c_scale->get(15), {0, 0}, 999);
    window->DrawList->AddCircleFilled(check_bb.Min + ImVec2(c_scale->get(2), 0) + ImVec2(check_bb.GetHeight() / 2 + (check_bb.GetWidth() / 2 - c_scale->get(4)) * cb_anim->second.anim.value, check_bb.GetHeight() / 2), (check_bb.GetHeight() / 2.2f), static_cast<ImColor>(c_utils->process_alpha(cb_anim->second.circle.value, style.Alpha)), 999);

    //window->DrawList->Flags |= ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines;

    // --- //

    cb_anim->second.outline.interpolate(ImColor(28, 28, 28).Value, ImColor(24, 24, 24).Value, hovered || IsItemActive());

    window->DrawList->AddRect(total_bb.Min, total_bb.Max, static_cast<ImColor>(c_utils->process_alpha(cb_anim->second.outline.value, style.Alpha)), style.FrameRounding);

    ImVec2 label_pos = ImVec2(total_bb.Min.x + style.FramePadding.x * 2, total_bb.Min.y + style.FramePadding.y * 3);

    cb_anim->second.text.interpolate(ImColor(255, 255, 255).Value, ImColor(150, 150, 150).Value, *v || hovered);

    PushStyleColor(ImGuiCol_Text, cb_anim->second.text.value);
    if (label_size.x > 0.0f)
        RenderText(label_pos, label);
    PopStyleColor();

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));

    auto backup_pos = GetCursorPos();

    if (prov_color2)
    {
        SetCursorScreenPos({ total_bb.Max.x - style.FramePadding.x - 48 - GetFrameHeight() + 4, total_bb.GetCenter().y - ((GetFrameHeight() - 4) / 2)});

        call_colorpicker_internal(label-1, reinterpret_cast<float*>(prov_color2), 0);
    }

    if (prov_color)
    {
        SetCursorScreenPos({ total_bb.Max.x - style.FramePadding.x - 8 - GetFrameHeight() + 4, total_bb.GetCenter().y - ((GetFrameHeight() - 4) / 2)});

        call_colorpicker_internal(label, reinterpret_cast<float*>(prov_color), 0);
    }

    SetCursorPos(backup_pos);

    return pressed;
}


bool custom_elements_t::toggle(const char* label, bool* v, ImVec4* prov_color)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    const float square_sz = label_size.y;
    const ImVec2 pos = window->DC.CursorPos;
    const ImRect total_bb(
        pos,
        pos + ImVec2(calculate_width(), label_size.y + style.FramePadding.y * 6.0f)
    );

    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id))
    {
        IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
        return false;
    }

    struct cb_data {
        animation anim, hov{};
        animation_vec4 col, col2{};
        animation_vec4 outline, shadow, circle{};
        animation_vec4 text{};
    };

    static std::map < ImGuiID, cb_data > anim2;
    auto cb_anim = anim2.find(id);
    if (cb_anim == anim2.end())
    {
        anim2.insert({ id, {} });
        cb_anim = anim2.find(id);
    }

    if (*v)
        cb_anim->second.anim.update(1);
    else
        cb_anim->second.anim.update(0);

    const ImVec2& size = { square_sz * 2, square_sz };

    float size_off{};
    if (prov_color)
        size_off = 12 + GetFrameHeight() - 4;

    const ImRect check_bb{
        ImVec2(total_bb.Max.x - size.x - style.FramePadding.x * 2 + c_scale->get(3) - size_off, total_bb.Min.y + total_bb.GetHeight() / 2 - size.y / 2),
        ImVec2(total_bb.Max.x - style.FramePadding.x * 2 + c_scale->get(3) - size_off, total_bb.Min.y + total_bb.GetHeight() / 2 + size.y / 2),
    };

    const ImRect touch_bb{
        total_bb.Min, check_bb.Max
    };

    bool hovered, held;
    bool pressed = ButtonBehavior(touch_bb, id, &hovered, &held);
    if (pressed)
    {
        *v = !(*v);
        MarkItemEdited(id);
    }

    if (hovered)
        cb_anim->second.hov.update(1);
    else
        cb_anim->second.hov.update(0);

    cb_anim->second.shadow.interpolate(c_utils->get_accent_imc(), ImColor(32, 32, 32).Value, * v);
    cb_anim->second.circle.interpolate(c_utils->get_accent_imc(), ImColor(32, 32, 32).Value, *v);

    cb_anim->second.col.interpolate(c_utils->get_accent_imc().Value, ImColor(24, 24, 24).Value, *v);

    cb_anim->second.col2.interpolate(c_utils->get_accent_imc(style.Alpha, 0.5f).Value, ImColor(24, 24, 24).Value, *v);

    window->DrawList->AddRectFilled(total_bb.Min, total_bb.Max, static_cast<ImColor>(c_utils->process_alpha(ImColor(18, 18, 18), style.Alpha)), style.FrameRounding);

    // toggle //

    //window->DrawList->Flags &= ~(ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines);

    window->DrawList->AddRectFilled(check_bb.Min + ImVec2(c_scale->get(3), c_scale->get(2)), check_bb.Max - ImVec2(c_scale->get(3), c_scale->get(2)), static_cast<ImColor>(c_utils->process_alpha(cb_anim->second.col2.value, style.Alpha)), c_scale->get(20));

   // window->DrawList->AddRect(check_bb.Min + ImVec2(3, 2), check_bb.Max - ImVec2(3, 2), static_cast<ImColor>(c_utils->process_alpha(cb_anim->second.col.value, style.Alpha)), 20);

    window->DrawList->AddShadowCircle(check_bb.Min + ImVec2(c_scale->get(2), 0) + ImVec2(check_bb.GetHeight() / 2 + (check_bb.GetWidth() / 2 - c_scale->get(4)) * cb_anim->second.anim.value, check_bb.GetHeight() / 2), (check_bb.GetHeight() / 2.2f), static_cast<ImColor>(c_utils->process_alpha(cb_anim->second.shadow.value, cb_anim->second.hov.value * style.Alpha)), c_scale->get(15), {0, 0}, 999);
    window->DrawList->AddCircleFilled(check_bb.Min + ImVec2(c_scale->get(2), 0) + ImVec2(check_bb.GetHeight() / 2 + (check_bb.GetWidth() / 2 - c_scale->get(4)) * cb_anim->second.anim.value, check_bb.GetHeight() / 2), (check_bb.GetHeight() / 2.2f), static_cast<ImColor>(c_utils->process_alpha(cb_anim->second.circle.value, style.Alpha)), 999);

    //window->DrawList->Flags |= ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines;

    // --- //

    cb_anim->second.outline.interpolate(ImColor(28, 28, 28).Value, ImColor(24, 24, 24).Value, hovered || IsItemActive());

    window->DrawList->AddRect(total_bb.Min, total_bb.Max, static_cast<ImColor>(c_utils->process_alpha(cb_anim->second.outline.value, style.Alpha)), style.FrameRounding);

    ImVec2 label_pos = ImVec2(total_bb.Min.x + style.FramePadding.x * 2, total_bb.Min.y + style.FramePadding.y * 3);

    cb_anim->second.text.interpolate(ImColor(255, 255, 255).Value, ImColor(150, 150, 150).Value, *v || hovered);

    PushStyleColor(ImGuiCol_Text, cb_anim->second.text.value);
    if (label_size.x > 0.0f)
        RenderText(label_pos, label);
    PopStyleColor();

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));

    auto backup_pos = GetCursorPos();

    if (prov_color) {

        SetCursorScreenPos({ total_bb.Max.x - style.FramePadding.x - 8 - GetFrameHeight() + 4, total_bb.GetCenter().y - ((GetFrameHeight() - 4) / 2)});

        call_colorpicker_internal(label, reinterpret_cast<float*>(prov_color), 0);
    }

    SetCursorPos(backup_pos);

    return pressed;
}

bool internal_scalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const float w = calculate_width();

    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    const ImRect frame_bb(
        window->DC.CursorPos + ImVec2(0, label_size.y + style.FramePadding.y * 2.0f) + ImVec2(0, c_scale->get(6)),
        window->DC.CursorPos + ImVec2(w, label_size.y * 2 + style.FramePadding.y * 7.0f) - ImVec2(0, c_scale->get(6))
    );

    const ImRect slider_bb(
        frame_bb.Min + ImVec2(c_scale->get(6), c_scale->get(1)),
        frame_bb.Max - ImVec2(c_scale->get(6), c_scale->get(6))
    );

    const ImRect total_bb(
        window->DC.CursorPos,
        frame_bb.Max
    );

    static struct slider_state {
        animation anim;
        animation_vec4 outline, shadow, text{};
    };

    static std::map < ImGuiID, slider_state > anim;
    auto it_anim = anim.find(id);
    if (it_anim == anim.end())
    {
        anim.insert({ id, {} });
        it_anim = anim.find(id);
    }

    const bool temp_input_allowed = (flags & ImGuiSliderFlags_NoInput) == 0;
    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id, &frame_bb, temp_input_allowed ? ImGuiItemFlags_Inputable : 0))
        return false;

    // Default format string when passing NULL
    if (format == NULL)
        format = DataTypeGetInfo(data_type)->PrintFmt;

    const bool hovered = ItemHoverable(total_bb, id, 0);
    // Tabbing or CTRL-clicking on Slider turns it into an input box
    const bool clicked = hovered && IsMouseClicked(0, id);
    const bool make_active = (clicked || g.NavActivateId == id);
    if (make_active && clicked)
        SetKeyOwner(ImGuiKey_MouseLeft, id);

    if (make_active)
    {
        SetActiveID(id, window);
        SetFocusID(id, window);
        FocusWindow(window);
        g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
    }

    // Draw frame
    const ImU32 frame_col = GetColorU32(g.ActiveId == id ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);

    // Slider behavior
    ImRect grab_bb;
    const bool value_changed = SliderBehavior(slider_bb, id, data_type, p_data, p_min, p_max, format, flags, &grab_bb);
    if (value_changed)
        MarkItemEdited(id);

    window->DrawList->AddRectFilled(total_bb.Min, total_bb.Max, static_cast<ImColor>(c_utils->process_alpha(ImColor(18, 18, 18).Value, style.Alpha)), style.FrameRounding);

    window->DrawList->AddRect(total_bb.Min, total_bb.Max, static_cast<ImColor>(it_anim->second.outline.value), style.FrameRounding);

    window->DrawList->AddRectFilled(slider_bb.Min + ImVec2(c_scale->get(3), c_scale->get(3)), slider_bb.Max - ImVec2(c_scale->get(1), c_scale->get(3)), static_cast<ImColor>(c_utils->process_alpha(ImColor(24, 24, 24).Value, style.Alpha)), style.FrameRounding);
    // Render grab

    const float& width = grab_bb.Max.x - slider_bb.Min.x;

    it_anim->second.anim.update(width);

    if (grab_bb.Max.x > grab_bb.Min.x) {
        it_anim->second.shadow.interpolate(c_utils->get_accent_imc().Value, ImColor(18, 18, 18).Value, IsItemActive() || value_changed);
        window->DrawList->AddShadowRect(slider_bb.Min + ImVec2(3, 3), ImVec2(slider_bb.Min.x + it_anim->second.anim.value, slider_bb.Max.y) - ImVec2(0, 3), static_cast<ImColor>(c_utils->process_alpha(it_anim->second.shadow.value, style.Alpha)), c_scale->get(20), { 0, 0 }, 20);
        window->DrawList->AddRectFilled(slider_bb.Min + ImVec2(3, 3), ImVec2(slider_bb.Min.x + it_anim->second.anim.value, slider_bb.Max.y) - ImVec2(0, 3), c_utils->get_accent_imc(style.Alpha), style.FrameRounding);
    }

    it_anim->second.outline.interpolate(ImColor(28, 28, 28).Value, ImColor(24, 24, 24).Value, value_changed || hovered || IsItemActive());

    window->DrawList->AddCircleFilled(ImVec2(slider_bb.Min.x + it_anim->second.anim.value - grab_bb.GetHeight() / 2, slider_bb.GetCenter().y), grab_bb.GetHeight() / 1.4f, c_utils->get_accent_imc(style.Alpha), 999);

    // Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
    char value_buf[64];
    const char* value_buf_end = value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, p_data, format);

    ImVec2 value_size = CalcTextSize(value_buf);

    it_anim->second.text.interpolate(ImColor(255, 255, 255).Value, ImColor(150, 150, 150).Value, hovered || IsItemActive() || value_changed);

    PushStyleColor(ImGuiCol_Text, it_anim->second.text.value);

    RenderText(ImVec2(total_bb.Max.x - style.FramePadding.x * 2 - value_size.x, total_bb.Min.y + style.FramePadding.y * 2), value_buf);
    RenderText(ImVec2(total_bb.Min.x + style.FramePadding.x * 2, total_bb.Min.y + style.FramePadding.y * 2), label);

    PopStyleColor();

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | (temp_input_allowed ? ImGuiItemStatusFlags_Inputable : 0));
    return value_changed;
}

bool custom_elements_t::slider_float(const char* label, float* v, float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
{
    return internal_scalar(label, ImGuiDataType_Float, v, &v_min, &v_max, format, flags);
}

bool custom_elements_t::slider_int(const char* label, int* v, int v_min, int v_max, const char* format, ImGuiSliderFlags flags)
{
    return internal_scalar(label, ImGuiDataType_S32, v, &v_min, &v_max, format, flags);
}

static float calc_max_height(int items_count)
{
    ImGuiContext& g = *GImGui;
    if (items_count <= 0)
        return FLT_MAX;
    return (g.FontSize + g.Style.ItemSpacing.y) * items_count - g.Style.ItemSpacing.y + (g.Style.WindowPadding.y * 2);
}

bool begin_combo(const char* label, const char* preview_value, float items_count, ImGuiComboFlags flags)
{
    static struct combo_state {
        animation anim;
        bool opened_combo = false, hovered = false;
        animation_vec4 preview_rect, outline, text{};
    };

    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();

    ImGuiNextWindowDataFlags backup_next_window_data_flags = g.NextWindowData.Flags;
    g.NextWindowData.ClearFlags(); // We behave like Begin() and need to consume those values
    if (window->SkipItems)
        return false;

    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    static std::map<ImGuiID, combo_state> anim;
    combo_state& state = anim[id];

    IM_ASSERT((flags & (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)) != (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)); // Can't use both flags together
    if (flags & ImGuiComboFlags_WidthFitPreview)
        IM_ASSERT((flags & (ImGuiComboFlags_NoPreview | (ImGuiComboFlags)ImGuiComboFlags_CustomPreview)) == 0);

    const float arrow_size = (flags & ImGuiComboFlags_NoArrowButton) ? 0.0f : GetFrameHeight();
    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    const float preview_width = ((flags & ImGuiComboFlags_WidthFitPreview) && (preview_value != NULL)) ? CalcTextSize(preview_value, NULL, true).x : 0.0f;
    const float w = calculate_width();
    //there were dragons

    const ImRect bb(
        window->DC.CursorPos + ImVec2(label_size.x + style.FramePadding.x * 4, style.FramePadding.x * 2),
        window->DC.CursorPos + ImVec2(w - style.FramePadding.x * 2, label_size.y + style.FramePadding.y * 7)
    );

    const ImRect total_bb(
        window->DC.CursorPos,
        bb.Max + ImVec2(style.FramePadding.x * 2, style.FramePadding.y * 3)
    );

    ItemSize(total_bb, style.FramePadding.y);

    if (!ItemAdd(bb, id, &bb)) return false;

    bool total_hovered = ItemHoverable(total_bb, id, 0);
    bool hovered, held, pressed = ButtonBehavior(bb, id, &hovered, &held);

    const ImGuiID popup_id = ImHashStr("##ComboPopup", 0, id);

    state.text.interpolate(ImColor(255, 255, 255).Value, ImColor(150, 150, 150).Value, hovered || state.opened_combo || state.hovered || IsItemActive() || total_hovered || held || pressed);

    // Render shape
    const ImU32 frame_col = GetColorU32(hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
    const float value_x2 = ImMax(bb.Min.x, bb.Max.x - arrow_size);

    window->DrawList->AddRectFilled(total_bb.Min, total_bb.Max, static_cast<ImColor>(c_utils->process_alpha(ImColor(18, 18, 18).Value, style.Alpha)), style.FrameRounding);

    window->DrawList->AddRectFilled(bb.Min, bb.Max, static_cast<ImColor>(c_utils->process_alpha(ImColor(20, 20, 20).Value, style.Alpha)), style.FrameRounding);
    //window->DrawList->AddRectFilled(bb.Min, bb.Max, static_cast<ImColor>(c_utils->process_alpha(ImColor(20, 20, 20).Value, style.Alpha)), 3);

    PushStyleColor(ImGuiCol_Text, state.text.value);

    // Render preview and label
    if (preview_value != NULL && !(flags & ImGuiComboFlags_NoPreview))
    {
        RenderTextClipped(ImVec2(bb.Min.x + style.FramePadding.x * 2, bb.GetCenter().y - CalcTextSize(preview_value).y / 2), ImVec2(bb.Max.x, bb.Max.y), preview_value, NULL, NULL);
    }

    if (pressed)
        state.opened_combo = !state.opened_combo;

    state.outline.interpolate(ImColor(28, 28, 28).Value, ImColor(24, 24, 24).Value, state.opened_combo || pressed || hovered || total_hovered);
    state.preview_rect.interpolate(c_utils->get_accent_imv4(GetStyle().Alpha), !hovered ? ImColor(24, 24, 24).Value : ImColor(28, 28, 28).Value, state.opened_combo || pressed);
    //if (hovered && g.IO.MouseClicked[0] || state.opened_combo && g.IO.MouseClicked[0] && !state.hovered) state.opened_combo = !state.opened_combo;

    if (label_size.x > 0)
        RenderText(ImVec2(total_bb.Min.x + style.FramePadding.x * 2, bb.GetCenter().y - label_size.y / 2), label);

    PopStyleColor();

    window->DrawList->AddRectFilledMultiColor(bb.Min, { bb.Max.x - style.FrameRounding, bb.Max.y }, ImColor(20, 20, 20, 0), static_cast<ImColor>(c_utils->process_alpha(ImColor(20, 20, 20).Value, style.Alpha)), static_cast<ImColor>(c_utils->process_alpha(ImColor(20, 20, 20).Value, style.Alpha)), ImColor(20, 20, 20, 0));
    window->DrawList->AddRectFilled({ bb.Max.x - style.FrameRounding, bb.Min.y }, bb.Max, static_cast<ImColor>(c_utils->process_alpha(ImColor(20, 20, 20).Value, style.Alpha)), style.FrameRounding, ImDrawFlags_RoundCornersRight);

    window->DrawList->AddRect(bb.Min, bb.Max, static_cast<ImColor>(state.preview_rect.value), style.FrameRounding);

    window->DrawList->AddRect(total_bb.Min, total_bb.Max, static_cast<ImColor>(state.outline.value), style.FrameRounding);

    // Custom preview
    if (flags & ImGuiComboFlags_CustomPreview)
    {
        g.ComboPreviewData.PreviewRect = ImRect(bb.Min.x, bb.Min.y, value_x2, bb.Max.y);
        IM_ASSERT(preview_value == NULL || preview_value[0] == 0);
        preview_value = NULL;
    }

    g.NextWindowData.Flags = backup_next_window_data_flags;

    if (!IsRectVisible(bb.Min, bb.Max + ImVec2(0, c_scale->get(2))))
    {
        state.opened_combo = false;
        state.anim.value = 0.f;
    }

    const ImVec2& arr_size = c_widgets->arrow_elem.font->CalcTextSizeA(c_scale->get(21), FLT_MAX, -1, (state.opened_combo ? c_widgets->arrow_elem.glyph_up : c_widgets->arrow_elem.glyph));
    window->DrawList->AddText(c_widgets->arrow_elem.font, c_scale->get(21), { bb.Max.x - arr_size.x - style.FramePadding.x * 2, bb.GetCenter().y - arr_size.y / 2 - 2 }, static_cast<ImColor>(c_utils->process_alpha(state.text.value, style.Alpha)), state.opened_combo ? c_widgets->arrow_elem.glyph_up : c_widgets->arrow_elem.glyph);

    if (!state.opened_combo && state.anim.value < 2.f) return false;

    state.anim.value = ImLerp(state.anim.value, state.opened_combo ? c_scale->get(12) + c_scale->get(10) + (c_scale->get(24) * items_count) : 0.f, g.IO.DeltaTime * 12.f);
    // This is essentially a specialized version of BeginPopupEx()
    char name[16];
    ImFormatString(name, IM_ARRAYSIZE(name), "##Combo_%02d", g.BeginComboDepth); // Recycle windows based on depth

    ImGui::SetNextWindowPos(ImVec2(bb.Min.x, bb.Max.y + c_scale->get(5)));

    ImGui::SetNextWindowSize(ImVec2(bb.GetWidth(), state.anim.value));

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoScrollWithMouse;

    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(c_scale->get(15), c_scale->get(15)));
    PushStyleVar(ImGuiStyleVar_WindowBorderSize, c_scale->get(1));
    PushStyleVar(ImGuiStyleVar_WindowRounding, c_scale->get(3));
    PushStyleColor(ImGuiCol_Border, ImColor(24, 24, 24).Value);

    auto ret = Begin(label, NULL, window_flags);

    PopStyleColor();
    PopStyleVar(3);

    state.hovered = IsWindowHovered();

    //if (multi && state.hovered && g.IO.MouseClicked[0]) state.opened_combo = false;

    ImRect nigger_bb(
        bb.Min,
        bb.Max + ImVec2(0, 5 + state.anim.value)
    );


    auto mouse_pos = ImGui::GetMousePos();

    bool hovered_outside{ true };

    /* -> check if mouse is on combo popup */ {
        auto area_begin = nigger_bb.Min;
        auto area_end = nigger_bb.Max;
        for (int i = 0; i < area_end.y - area_begin.y; i++) {
            for (int n = 0; n < area_end.x - area_begin.x; n++) {
                if (mouse_pos.y == area_begin.y + i && mouse_pos.x == area_begin.x + n) {
                    hovered_outside = false;
                }
            }
        }
    }

    if (hovered_outside && state.opened_combo && g.IO.MouseClicked[0])
        state.opened_combo = false;

    return ret;
}

// Getter for the old Combo() API: const char*[]
static const char* Items_ArrayGetter(void* data, int idx)
{
    const char* const* items = (const char* const*)data;
    return items[idx];
}

// Getter for the old Combo() API: "item1\0item2\0item3\0"
static const char* Items_SingleStringGetter(void* data, int idx)
{
    const char* items_separated_by_zeros = (const char*)data;
    int items_count = 0;
    const char* p = items_separated_by_zeros;
    while (*p)
    {
        if (idx == items_count)
            break;
        p += strlen(p) + 1;
        items_count++;
    }
    return *p ? p : NULL;
}


// Old API, prefer using BeginCombo() nowadays if you can.
bool custom_elements_t::combo(const char* label, int* current_item, const char* (*getter)(void* user_data, int idx), void* user_data, int items_count, int popup_max_height_in_items)
{
    ImGuiContext& g = *GImGui;

    // Call the getter to obtain the preview string which is a parameter to BeginCombo()
    const char* preview_value = NULL;
    if (*current_item >= 0 && *current_item < items_count)
        preview_value = getter(user_data, *current_item);

    if (popup_max_height_in_items != -1 && !(g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSizeConstraint))
        SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, calc_max_height(popup_max_height_in_items)));

    if (!begin_combo(label, preview_value, items_count, ImGuiComboFlags_None))
        return false;

    bool value_changed = false;
    for (int i = 0; i < items_count; i++)
    {
        const char* item_text = getter(user_data, i);
        if (item_text == NULL)
            item_text = "*Unknown item*";

        PushID(i);
        const bool item_selected = (i == *current_item);
        if (selectable(item_text, item_selected) && *current_item != i)
        {
            value_changed = true;
            *current_item = i;
        }
        if (item_selected)
            SetItemDefaultFocus();
        PopID();
    }

    End();
    if (value_changed)
        MarkItemEdited(g.LastItemData.ID);

    return value_changed;
}

// Combo box helper allowing to pass an array of strings.
bool custom_elements_t::combo(const char* label, int* current_item, const char* const items[], int items_count, int height_in_items)
{
    const bool value_changed = combo(label, current_item, Items_ArrayGetter, (void*)items, items_count, height_in_items);
    return value_changed;
}

// Combo box helper allowing to pass all items in a single string literal holding multiple zero-terminated items "item1\0item2\0"
bool custom_elements_t::combo(const char* label, int* current_item, const char* items_separated_by_zeros, int height_in_items)
{
    int items_count = 0;
    const char* p = items_separated_by_zeros;       // FIXME-OPT: Avoid computing this, or at least only when combo is open
    while (*p)
    {
        p += strlen(p) + 1;
        items_count++;
    }
    bool value_changed = combo(label, current_item, Items_SingleStringGetter, (void*)items_separated_by_zeros, items_count, height_in_items);
    return value_changed;
}

struct ImGuiGetNameFromIndexOldToNewCallbackData { void* UserData; bool (*OldCallback)(void*, int, const char**); };
static const char* ImGuiGetNameFromIndexOldToNewCallback(void* user_data, int idx)
{
    ImGuiGetNameFromIndexOldToNewCallbackData* data = (ImGuiGetNameFromIndexOldToNewCallbackData*)user_data;
    const char* s = NULL;
    data->OldCallback(data->UserData, idx, &s);
    return s;
}

bool custom_elements_t::combo(const char* label, int* current_item, bool (*old_getter)(void*, int, const char**), void* user_data, int items_count, int popup_max_height_in_items)
{
    ImGuiGetNameFromIndexOldToNewCallbackData old_to_new_data = { user_data, old_getter };
    return combo(label, current_item, ImGuiGetNameFromIndexOldToNewCallback, &old_to_new_data, items_count, popup_max_height_in_items);
}

bool custom_elements_t::selectable(const char* label, bool selected, ImGuiSelectableFlags flags, const ImVec2& size_arg)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    // Submit label or explicit size to ItemSize(), whereas ItemAdd() will submit a larger/spanning rectangle.
    ImGuiID id = window->GetID(label);
    ImVec2 label_size = CalcTextSize(label, NULL, true);
    ImVec2 size(size_arg.x != 0.0f ? size_arg.x : label_size.x, size_arg.y != 0.0f ? size_arg.y : label_size.y);
    ImVec2 pos = window->DC.CursorPos;
    pos.y += window->DC.CurrLineTextBaseOffset;
    ItemSize(size, 0.0f);

    static struct selectable_state {
        animation_vec4 text;
        animation_vec4 bg;
        animation_vec4 circle;
        animation anim;
    };

    static std::map < ImGuiID, selectable_state > anim2;
    auto cb_anim = anim2.find(id);
    if (cb_anim == anim2.end())
    {
        anim2.insert({ id, {ImColor(150, 150, 150).Value, ImColor(20, 20, 20, 0).Value} });
        cb_anim = anim2.find(id);
    }

    // Fill horizontal space
    // We don't support (size < 0.0f) in Selectable() because the ItemSpacing extension would make explicitly right-aligned sizes not visibly match other widgets.
    const bool span_all_columns = (flags & ImGuiSelectableFlags_SpanAllColumns) != 0;
    const float min_x = span_all_columns ? window->ParentWorkRect.Min.x : pos.x;
    const float max_x = span_all_columns ? window->ParentWorkRect.Max.x : window->WorkRect.Max.x;
    if (size_arg.x == 0.0f || (flags & ImGuiSelectableFlags_SpanAvailWidth))
        size.x = ImMax(label_size.x, max_x - min_x);

    // Text stays at the submission position, but bounding box may be extended on both sides
    const ImVec2 text_min = pos + ImVec2(c_scale->get(15) * cb_anim->second.anim.value, 0);
    const ImVec2 text_max = ImVec2(min_x + size.x, pos.y + size.y) + ImVec2(c_scale->get(15) * cb_anim->second.anim.value, 0);

    // Selectables are meant to be tightly packed together with no click-gap, so we extend their box to cover spacing between selectable.
    ImRect bb(min_x, pos.y, text_max.x, text_max.y);
    if ((flags & ImGuiSelectableFlags_NoPadWithHalfSpacing) == 0)
    {
        const float spacing_x = span_all_columns ? 0.0f : style.ItemSpacing.x;
        const float spacing_y = style.ItemSpacing.y;
        const float spacing_L = IM_TRUNC(spacing_x * 0.50f);
        const float spacing_U = IM_TRUNC(spacing_y * 0.50f);
        bb.Min.x -= spacing_L;
        bb.Min.y -= spacing_U;
        bb.Max.x += (spacing_x - spacing_L);
        bb.Max.y += (spacing_y - spacing_U);
    }
    //if (g.IO.KeyCtrl) { GetForegroundDrawList()->AddRect(bb.Min, bb.Max, IM_COL32(0, 255, 0, 255)); }

    // Modify ClipRect for the ItemAdd(), faster than doing a PushColumnsBackground/PushTableBackgroundChannel for every Selectable..
    const float backup_clip_rect_min_x = window->ClipRect.Min.x;
    const float backup_clip_rect_max_x = window->ClipRect.Max.x;
    if (span_all_columns)
    {
        window->ClipRect.Min.x = window->ParentWorkRect.Min.x;
        window->ClipRect.Max.x = window->ParentWorkRect.Max.x;
    }

    const bool disabled_item = (flags & ImGuiSelectableFlags_Disabled) != 0;
    const bool item_add = ItemAdd(bb, id, NULL, disabled_item ? ImGuiItemFlags_Disabled : ImGuiItemFlags_None);
    if (span_all_columns)
    {
        window->ClipRect.Min.x = backup_clip_rect_min_x;
        window->ClipRect.Max.x = backup_clip_rect_max_x;
    }

    if (!item_add)
        return false;

    const bool disabled_global = (g.CurrentItemFlags & ImGuiItemFlags_Disabled) != 0;
    if (disabled_item && !disabled_global) // Only testing this as an optimization
        BeginDisabled();

    // FIXME: We can standardize the behavior of those two, we could also keep the fast path of override ClipRect + full push on render only,
    // which would be advantageous since most selectable are not selected.
    if (span_all_columns)
    {
        if (g.CurrentTable)
            TablePushBackgroundChannel();
        else if (window->DC.CurrentColumns)
            PushColumnsBackground();
    }

    // We use NoHoldingActiveID on menus so user can click and _hold_ on a menu then drag to browse child entries
    ImGuiButtonFlags button_flags = 0;
    if (flags & ImGuiSelectableFlags_NoHoldingActiveID) { button_flags |= ImGuiButtonFlags_NoHoldingActiveId; }
    if (flags & ImGuiSelectableFlags_NoSetKeyOwner) { button_flags |= ImGuiButtonFlags_NoSetKeyOwner; }
    if (flags & ImGuiSelectableFlags_SelectOnClick) { button_flags |= ImGuiButtonFlags_PressedOnClick; }
    if (flags & ImGuiSelectableFlags_SelectOnRelease) { button_flags |= ImGuiButtonFlags_PressedOnRelease; }
    if (flags & ImGuiSelectableFlags_AllowDoubleClick) { button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick; }
    if ((flags & ImGuiSelectableFlags_AllowOverlap) || (g.LastItemData.InFlags & ImGuiItemFlags_AllowOverlap)) { button_flags |= ImGuiButtonFlags_AllowOverlap; }

    const bool was_selected = selected;
    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, button_flags);

    // Auto-select when moved into
    // - This will be more fully fleshed in the range-select branch
    // - This is not exposed as it won't nicely work with some user side handling of shift/control
    // - We cannot do 'if (g.NavJustMovedToId != id) { selected = false; pressed = was_selected; }' for two reasons
    //   - (1) it would require focus scope to be set, need exposing PushFocusScope() or equivalent (e.g. BeginSelection() calling PushFocusScope())
    //   - (2) usage will fail with clipped items
    //   The multi-select API aim to fix those issues, e.g. may be replaced with a BeginSelection() API.
    if ((flags & ImGuiSelectableFlags_SelectOnNav) && g.NavJustMovedToId != 0 && g.NavJustMovedToFocusScopeId == g.CurrentFocusScopeId)
        if (g.NavJustMovedToId == id)
            selected = pressed = true;

    // Update NavId when clicking or when Hovering (this doesn't happen on most widgets), so navigation can be resumed with gamepad/keyboard
    if (pressed || (hovered && (flags & ImGuiSelectableFlags_SetNavIdOnHover)))
    {
        if (!g.NavDisableMouseHover && g.NavWindow == window && g.NavLayer == window->DC.NavLayerCurrent)
        {
            SetNavID(id, window->DC.NavLayerCurrent, g.CurrentFocusScopeId, WindowRectAbsToRel(window, bb)); // (bb == NavRect)
            g.NavDisableHighlight = true;
        }
    }
    if (pressed)
        MarkItemEdited(id);

    // In this branch, Selectable() cannot toggle the selection so this will never trigger.
    if (selected != was_selected) //-V547
        g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_ToggledSelection;

    cb_anim->second.circle.interpolate(c_utils->process_alpha(c_utils->get_accent_imv4(), g.Style.Alpha), c_utils->process_alpha(c_utils->get_accent_imv4(), 0), selected);

    if (selected) {
        cb_anim->second.text.update(c_utils->get_accent_imc().Value);
        cb_anim->second.bg.update(ImColor(20, 20, 20).Value);
        cb_anim->second.anim.update(1);
    }
    else {
        cb_anim->second.text.update(ImColor(150, 150, 150).Value);
        cb_anim->second.bg.update(ImColor(20, 20, 20, 0).Value);
        cb_anim->second.anim.update(0);
    }

    if (span_all_columns)
    {
        if (window->DC.CurrentColumns)
            PopColumnsBackground();
    }

    const float& circle_rad = bb.GetHeight() / 6;
    window->DrawList->AddShadowCircle(ImVec2(pos.x, bb.GetCenter().y) + ImVec2(circle_rad, 1), circle_rad, static_cast<ImColor>(cb_anim->second.circle.value), c_scale->get(20), { 0, 0 }, 999);
    window->DrawList->AddCircleFilled(ImVec2(pos.x, bb.GetCenter().y) + ImVec2(circle_rad, 1), circle_rad, static_cast<ImColor>(cb_anim->second.circle.value), 999);

    //window->DrawList->AddRectFilled(bb.Min, bb.Max, static_cast<ImColor>(cb_anim->second.bg.value), 2);

    PushStyleColor(ImGuiCol_Text, cb_anim->second.text.value);
    RenderTextClipped(text_min, text_max, label, NULL, &label_size, style.SelectableTextAlign, &bb);
    PopStyleColor();

    // Automatically close popups
    if (pressed && (window->Flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiSelectableFlags_DontClosePopups) && !(g.LastItemData.InFlags & ImGuiItemFlags_SelectableDontClosePopup))
        CloseCurrentPopup();

    if (disabled_item && !disabled_global)
        EndDisabled();

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    return pressed; //-V1020*/
}

bool custom_elements_t::selectable(const char* label, bool* p_selected, ImGuiSelectableFlags flags, const ImVec2& size_arg)
{
    if (selectable(label, *p_selected, flags, size_arg))
    {
        *p_selected = !*p_selected;
        return true;
    }
    return false;
}

void custom_elements_t::multi_combo(const char* label, bool* combos[], const char* items[], int items_count) {
    std::vector<std::string> vec;
    static std::string preview;
    for (int i = 0, j = 0; i < items_count; i++) {
        if ((*combos)[i]) {
            vec.push_back(items[i]);
            if (j > 1) preview = vec.at(0) + std::string((", ")) + vec.at(1) + std::string((", ..."));
            else if (j) preview += std::string((", ")) + (std::string)items[i];
            else preview = items[i];
            j++;
        }
    }
    if (begin_combo(label, preview.c_str(), items_count, 0)) {
        for (int i = 0; i < items_count; i++) {
            selectable(items[i], &((*combos)[i]), ImGuiSelectableFlags_DontClosePopups);
            if ((*combos)[i]) ImColor(60, 60, 60);
        }
        End();
    }
    preview = ("None");
}


bool call_colorpicker_internal(const char* label, float col[4], ImGuiColorEditFlags flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const float square_sz = GetFrameHeight() - 4;
    const char* label_display_end = FindRenderedTextEnd(label);
    float w_full = CalcItemWidth();
    g.NextItemData.ClearFlags();

    BeginGroup();
    PushID(label);
    const bool set_current_color_edit_id = (g.ColorEditCurrentID == 0);
    if (set_current_color_edit_id)
        g.ColorEditCurrentID = window->IDStack.back();

    // If we're not showing any slider there's no point in doing any HSV conversions
    const ImGuiColorEditFlags flags_untouched = flags;
    if (flags & ImGuiColorEditFlags_NoInputs)
        flags = (flags & (~ImGuiColorEditFlags_DisplayMask_)) | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoOptions;

    // Context menu: display and modify options (before defaults are applied)
    //if (!(flags & ImGuiColorEditFlags_NoOptions))
        //ColorEditOptionsPopup(col, flags);

    // Read stored options
    if (!(flags & ImGuiColorEditFlags_DisplayMask_))
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags_DisplayMask_);
    if (!(flags & ImGuiColorEditFlags_DataTypeMask_))
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags_DataTypeMask_);
    if (!(flags & ImGuiColorEditFlags_PickerMask_))
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags_PickerMask_);
    if (!(flags & ImGuiColorEditFlags_InputMask_))
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags_InputMask_);
    flags |= (g.ColorEditOptions & ~(ImGuiColorEditFlags_DisplayMask_ | ImGuiColorEditFlags_DataTypeMask_ | ImGuiColorEditFlags_PickerMask_ | ImGuiColorEditFlags_InputMask_));
    IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags_DisplayMask_)); // Check that only 1 is selected
    IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags_InputMask_));   // Check that only 1 is selected

    const bool alpha = true;
    const bool hdr = (flags & ImGuiColorEditFlags_HDR) != 0;
    const int components = alpha ? 4 : 3;
    const float w_button = (flags & ImGuiColorEditFlags_NoSmallPreview) ? 0.0f : (square_sz + style.ItemInnerSpacing.x);
    const float w_inputs = ImMax(w_full - w_button, 1.0f);
    w_full = w_inputs + w_button;

    // Convert to the formats we need
    float f[4] = { col[0], col[1], col[2], alpha ? col[3] : 1.0f };
    if ((flags & ImGuiColorEditFlags_InputHSV) && (flags & ImGuiColorEditFlags_DisplayRGB))
        ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
    else if ((flags & ImGuiColorEditFlags_InputRGB) && (flags & ImGuiColorEditFlags_DisplayHSV))
    {
        // Hue is lost when converting from grayscale rgb (saturation=0). Restore it.
        ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);
        ColorEditRestoreHS(col, &f[0], &f[1], &f[2]);
    }
    int i[4] = { IM_F32_TO_INT8_UNBOUND(f[0]), IM_F32_TO_INT8_UNBOUND(f[1]), IM_F32_TO_INT8_UNBOUND(f[2]), IM_F32_TO_INT8_UNBOUND(f[3]) };

    bool value_changed = false;
    bool value_changed_as_float = false;

    const ImVec2 pos = window->DC.CursorPos;

    const ImRect total_bb(
        pos,
        pos + ImVec2(square_sz, square_sz)
    );

    static struct picker_state {
        bool active = false;
        bool hovered = false;
        animation anim{};
        animation_vec4 outline, text{};
    };

    static std::map<ImGuiID, picker_state> anim_;
    picker_state& state = anim_[ImGui::GetID(label)];

    ImGuiWindow* picker_active_window = NULL;
    {
        PushStyleVar(ImGuiStyleVar_WindowPadding, { c_scale->get(10), c_scale->get(10) });

        const ImVec4 col_v4(col[0], col[1], col[2], alpha ? col[3] : 1.0f);
        if (color_button_internal("##ColorButton", col_v4, flags, { square_sz, square_sz }))
        {
            if (!(flags & ImGuiColorEditFlags_NoPicker))
            {
                // Store current color and open a picker
                g.ColorPickerRef = col_v4;
                OpenPopup("picker");

                SetNextWindowPos(g.LastItemData.Rect.GetBL());
            }
        }

        if (!(flags & ImGuiColorEditFlags_NoOptions)) OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);

        state.hovered = ItemHoverable(g.LastItemData.Rect, g.LastItemData.ID, NULL);
        if (state.hovered && g.IO.MouseClicked[0])
            state.active = true;

        if (state.active)
            state.anim.update(1);
        else
            state.anim.update(0);

        PushStyleVar(ImGuiStyleVar_Alpha, state.anim.value);

        if (state.active || state.anim.value > 0.01f) {
            SetNextWindowPos(g.LastItemData.Rect.GetBL());

            Begin("picker", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);
            {
                //if (BeginPopup("picker"))
                //{
                if (g.CurrentWindow->BeginCount == 1)
                {
                    picker_active_window = g.CurrentWindow;
                    ImGuiColorEditFlags picker_flags_to_forward = ImGuiColorEditFlags_DataTypeMask_ | ImGuiColorEditFlags_PickerMask_ | ImGuiColorEditFlags_InputMask_ | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_AlphaBar;
                    ImGuiColorEditFlags picker_flags = (flags_untouched & picker_flags_to_forward) | ImGuiColorEditFlags_DisplayMask_ | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_NoInputs;
                    SetNextItemWidth(square_sz * 12.0f); // Use 256 + bar sizes?
                    value_changed |= color_picker_internal("##picker", col, picker_flags, &g.ColorPickerRef.x);
                }

                auto mouse_pos = ImGui::GetMousePos();

                bool hovered_outside{ true };

                /* -> check if mouse is on combo popup */ {
                    const auto& area_begin = GetWindowPos();
                    const auto& area_end = GetWindowPos() + GetWindowSize();

                    for (int i = 0; i < area_end.y - area_begin.y; i++) {
                        for (int n = 0; n < area_end.x - area_begin.x; n++) {
                            if (mouse_pos.y == area_begin.y + i && mouse_pos.x == area_begin.x + n) {
                                hovered_outside = false;
                            }
                        }
                    }
                }

                if (hovered_outside && state.active && g.IO.MouseClicked[0] && state.active && state.anim.value > 0.8f)
                    state.active = false;

                End();
            }
        }

        PopStyleVar(2);
    }

    // Convert back
    if (value_changed && picker_active_window == NULL)
    {
        if (!value_changed_as_float)
            for (int n = 0; n < 4; n++)
                f[n] = i[n] / 255.0f;
        if ((flags & ImGuiColorEditFlags_DisplayHSV) && (flags & ImGuiColorEditFlags_InputRGB))
        {
            g.ColorEditSavedHue = f[0];
            g.ColorEditSavedSat = f[1];
            ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
            g.ColorEditSavedID = g.ColorEditCurrentID;
            g.ColorEditSavedColor = ColorConvertFloat4ToU32(ImVec4(f[0], f[1], f[2], 0));
        }
        if ((flags & ImGuiColorEditFlags_DisplayRGB) && (flags & ImGuiColorEditFlags_InputHSV))
            ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);

        col[0] = f[0];
        col[1] = f[1];
        col[2] = f[2];
        if (alpha)
            col[3] = f[3];
    }

    if (set_current_color_edit_id)
        g.ColorEditCurrentID = 0;
    PopID();
    EndGroup();

    // When picker is being actively used, use its active id so IsItemActive() will function on ColorEdit4().
    if (picker_active_window && g.ActiveId != 0 && g.ActiveIdWindow == picker_active_window)
        g.LastItemData.ID = g.ActiveId;

    if (value_changed && g.LastItemData.ID != 0) // In case of ID collision, the second EndGroup() won't catch g.ActiveId
        MarkItemEdited(g.LastItemData.ID);

    return value_changed;
}

bool custom_elements_t::color_edit_4(const char* label, float col[4], ImGuiColorEditFlags flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const float square_sz = GetFrameHeight() - 4;
    const char* label_display_end = FindRenderedTextEnd(label);
    float w_full = CalcItemWidth();
    g.NextItemData.ClearFlags();

    BeginGroup();
    PushID(label);
    const bool set_current_color_edit_id = (g.ColorEditCurrentID == 0);
    if (set_current_color_edit_id)
        g.ColorEditCurrentID = window->IDStack.back();

    // If we're not showing any slider there's no point in doing any HSV conversions
    const ImGuiColorEditFlags flags_untouched = flags;
    if (flags & ImGuiColorEditFlags_NoInputs)
        flags = (flags & (~ImGuiColorEditFlags_DisplayMask_)) | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoOptions;

    // Context menu: display and modify options (before defaults are applied)
    //if (!(flags & ImGuiColorEditFlags_NoOptions))
        //ColorEditOptionsPopup(col, flags);

    // Read stored options
    if (!(flags & ImGuiColorEditFlags_DisplayMask_))
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags_DisplayMask_);
    if (!(flags & ImGuiColorEditFlags_DataTypeMask_))
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags_DataTypeMask_);
    if (!(flags & ImGuiColorEditFlags_PickerMask_))
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags_PickerMask_);
    if (!(flags & ImGuiColorEditFlags_InputMask_))
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags_InputMask_);
    flags |= (g.ColorEditOptions & ~(ImGuiColorEditFlags_DisplayMask_ | ImGuiColorEditFlags_DataTypeMask_ | ImGuiColorEditFlags_PickerMask_ | ImGuiColorEditFlags_InputMask_));
    IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags_DisplayMask_)); // Check that only 1 is selected
    IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags_InputMask_));   // Check that only 1 is selected

    const bool alpha = true;
    const bool hdr = (flags & ImGuiColorEditFlags_HDR) != 0;
    const int components = alpha ? 4 : 3;
    const float w_button = (flags & ImGuiColorEditFlags_NoSmallPreview) ? 0.0f : (square_sz + style.ItemInnerSpacing.x);
    const float w_inputs = ImMax(w_full - w_button, 1.0f);
    w_full = w_inputs + w_button;

    // Convert to the formats we need
    float f[4] = { col[0], col[1], col[2], alpha ? col[3] : 1.0f };
    if ((flags & ImGuiColorEditFlags_InputHSV) && (flags & ImGuiColorEditFlags_DisplayRGB))
        ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
    else if ((flags & ImGuiColorEditFlags_InputRGB) && (flags & ImGuiColorEditFlags_DisplayHSV))
    {
        // Hue is lost when converting from grayscale rgb (saturation=0). Restore it.
        ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);
        ColorEditRestoreHS(col, &f[0], &f[1], &f[2]);
    }
    int i[4] = { IM_F32_TO_INT8_UNBOUND(f[0]), IM_F32_TO_INT8_UNBOUND(f[1]), IM_F32_TO_INT8_UNBOUND(f[2]), IM_F32_TO_INT8_UNBOUND(f[3]) };

    bool value_changed = false;
    bool value_changed_as_float = false;

    const ImVec2 pos = window->DC.CursorPos;

    const ImRect total_bb(
        pos,
        pos + ImVec2(calculate_width(), CalcTextSize(label).y + style.FramePadding.y * 6.0f)
    );

    static struct picker_state {
        bool active = false;
        bool hovered = false;
        animation anim{};
        animation_vec4 outline, text{};
    };

    const ImRect hover_rect(
        total_bb.Min,
        ImVec2(total_bb.Max.x - square_sz - style.FramePadding.x * 2, total_bb.Max.y)
    );
    bool total_hovered = ItemHoverable(hover_rect, GetID(label), NULL);

    static std::map<ImGuiID, picker_state> anim_;
    picker_state& state = anim_[ImGui::GetID(label)];

    state.outline.interpolate(ImColor(28, 28, 28).Value, ImColor(24, 24, 24).Value, total_hovered || state.hovered || IsItemActive());

    window->DrawList->AddRectFilled(total_bb.Min, total_bb.Max, static_cast<ImColor>(c_utils->process_alpha(ImColor(18, 18, 18), style.Alpha)), style.FrameRounding);

    window->DrawList->AddRect(total_bb.Min, total_bb.Max, static_cast<ImColor>(c_utils->process_alpha(state.outline.value, style.Alpha)), style.FrameRounding);

    ImGuiWindow* picker_active_window = NULL;
    {
        PushStyleVar(ImGuiStyleVar_WindowPadding, { c_scale->get(10), c_scale->get(10) });

        window->DC.CursorPos = ImVec2(total_bb.Max.x - square_sz - style.FramePadding.x * 2, total_bb.GetCenter().y - square_sz / 2);

        const ImVec4 col_v4(col[0], col[1], col[2], alpha ? col[3] : 1.0f);
        if (color_button_internal("##ColorButton", col_v4, flags, { square_sz, square_sz }))
        {
            if (!(flags & ImGuiColorEditFlags_NoPicker))
            {
                // Store current color and open a picker
                g.ColorPickerRef = col_v4;
                OpenPopup("picker");

                SetNextWindowPos(g.LastItemData.Rect.GetBL());
            }
        }

        if (!(flags & ImGuiColorEditFlags_NoOptions)) OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);

        state.hovered = ItemHoverable(g.LastItemData.Rect, g.LastItemData.ID, NULL);
        if (state.hovered && g.IO.MouseClicked[0])
            state.active = true;

        if (state.active)
            state.anim.update(1);
        else
            state.anim.update(0);

        PushStyleVar(ImGuiStyleVar_Alpha, state.anim.value);

        if (state.active || state.anim.value > 0.01f) {
            SetNextWindowPos(g.LastItemData.Rect.GetBL());

            Begin("picker", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);
            {
                //if (BeginPopup("picker"))
                //{
                if (g.CurrentWindow->BeginCount == 1)
                {
                    picker_active_window = g.CurrentWindow;
                    ImGuiColorEditFlags picker_flags_to_forward = ImGuiColorEditFlags_DataTypeMask_ | ImGuiColorEditFlags_PickerMask_ | ImGuiColorEditFlags_InputMask_ | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_AlphaBar;
                    ImGuiColorEditFlags picker_flags = (flags_untouched & picker_flags_to_forward) | ImGuiColorEditFlags_DisplayMask_ | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_NoInputs;
                    SetNextItemWidth(square_sz * 12.0f); // Use 256 + bar sizes?
                    value_changed |= color_picker_internal("##picker", col, picker_flags, &g.ColorPickerRef.x);
                }

                auto mouse_pos = ImGui::GetMousePos();

                bool hovered_outside{ true };

                /* -> check if mouse is on combo popup */ {
                    const auto& area_begin = GetWindowPos();
                    const auto& area_end = GetWindowPos() + GetWindowSize();

                    for (int i = 0; i < area_end.y - area_begin.y; i++) {
                        for (int n = 0; n < area_end.x - area_begin.x; n++) {
                            if (mouse_pos.y == area_begin.y + i && mouse_pos.x == area_begin.x + n) {
                                hovered_outside = false;
                            }
                        }
                    }
                }

                if (hovered_outside && state.active && g.IO.MouseClicked[0] && state.active && state.anim.value > 0.8f)
                    state.active = false;

                End();
            }
        }

        PopStyleVar(2);
    }

    state.text.interpolate(ImColor(255, 255, 255).Value, ImColor(150, 150, 150).Value, IsItemActive() || total_hovered || state.hovered || state.active);

    PushStyleColor(ImGuiCol_Text, state.text.value);

    if (label != label_display_end && !(flags & ImGuiColorEditFlags_NoLabel))
    {
        // Position not necessarily next to last submitted button (e.g. if style.ColorButtonPosition == ImGuiDir_Left),
        // but we need to use SameLine() to setup baseline correctly. Might want to refactor SameLine() to simplify this.
        RenderText({ total_bb.Min.x + style.FramePadding.x * 2, total_bb.GetCenter().y - CalcTextSize(label).y / 2 }, label);
    }

    PopStyleColor();

    // Convert back
    if (value_changed && picker_active_window == NULL)
    {
        if (!value_changed_as_float)
            for (int n = 0; n < 4; n++)
                f[n] = i[n] / 255.0f;
        if ((flags & ImGuiColorEditFlags_DisplayHSV) && (flags & ImGuiColorEditFlags_InputRGB))
        {
            g.ColorEditSavedHue = f[0];
            g.ColorEditSavedSat = f[1];
            ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
            g.ColorEditSavedID = g.ColorEditCurrentID;
            g.ColorEditSavedColor = ColorConvertFloat4ToU32(ImVec4(f[0], f[1], f[2], 0));
        }
        if ((flags & ImGuiColorEditFlags_DisplayRGB) && (flags & ImGuiColorEditFlags_InputHSV))
            ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);

        col[0] = f[0];
        col[1] = f[1];
        col[2] = f[2];
        if (alpha)
            col[3] = f[3];
    }

    if (set_current_color_edit_id)
        g.ColorEditCurrentID = 0;
    PopID();
    EndGroup();

    // When picker is being actively used, use its active id so IsItemActive() will function on ColorEdit4().
    if (picker_active_window && g.ActiveId != 0 && g.ActiveIdWindow == picker_active_window)
        g.LastItemData.ID = g.ActiveId;

    if (value_changed && g.LastItemData.ID != 0) // In case of ID collision, the second EndGroup() won't catch g.ActiveId
        MarkItemEdited(g.LastItemData.ID);

    window->DC.CursorPos.y = total_bb.Max.y + style.ItemSpacing.y;

    return value_changed;
}


bool internal_btn(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;

    ImVec2 size = CalcItemSize(size_arg, calculate_width(), label_size.y + style.FramePadding.y * 6.0f);

    const ImRect bb(pos, pos + size);
    ItemSize(size, style.FramePadding.y);
    if (!ItemAdd(bb, id))
        return false;

    struct btn_data {
        animation_vec4 anim, outline, text{};
    };

    static std::map < ImGuiID, btn_data > anim2;
    auto bt_anim = anim2.find(id);
    if (bt_anim == anim2.end())
    {
        anim2.insert({ id, {} });
        bt_anim = anim2.find(id);
    }

    bt_anim->second.anim.update(c_utils->process_alpha(ImColor(18, 18, 18).Value, style.Alpha));

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

    if (pressed)
        bt_anim->second.anim.value = ImColor(18, 18, 18, 0).Value;

    // Render
    window->DrawList->AddRectFilled(bb.Min, bb.Max, static_cast<ImColor>(bt_anim->second.anim.value), 3);

    bt_anim->second.outline.interpolate(ImColor(28, 28, 28).Value, ImColor(24, 24, 24).Value, hovered || IsItemActive());

    window->DrawList->AddRect(bb.Min, bb.Max, static_cast<ImColor>(c_utils->process_alpha(bt_anim->second.outline.value, style.Alpha)), style.FrameRounding);

    bt_anim->second.text.interpolate(ImColor(255, 255, 255).Value, ImColor(150, 150, 150).Value, hovered || pressed || IsItemActive());

    PushStyleColor(ImGuiCol_Text, bt_anim->second.text.value);

    if (g.LogEnabled)
        LogSetNextTextDecoration("[", "]");
    RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

    PopStyleColor();

    // Automatically close popups
    //if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
    //    CloseCurrentPopup();

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    return pressed;
}

bool custom_elements_t::button(const char* label, const ImVec2& size_arg)
{
    return internal_btn(label, size_arg, ImGuiButtonFlags_None);
}

void custom_elements_t::custom_text(const char* v, ImFont* font) {
    PushFont(font);
    Text("%s", v);
    PopFont();
}

bool custom_elements_t::begin_listbox(const char* label, const ImVec2& size_arg)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    const ImGuiStyle& style = g.Style;
    const ImGuiID id = GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    // Size default to hold ~7.25 items.
    // Fractional number of items helps seeing that we can scroll down/up without looking at scrollbar.
    ImVec2 size = ImTrunc(CalcItemSize(size_arg, calculate_width(), GetTextLineHeightWithSpacing() * 7.25f + style.FramePadding.y * 2.0f));
    ImVec2 frame_size = ImVec2(size.x, ImMax(size.y, label_size.y));
    ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + frame_size);

    ImRect bb(frame_bb.Min, frame_bb.Max);
    // g.NextItemData.ClearFlags();

    auto mouse_pos = ImGui::GetMousePos();

    bool hovered{ false };

    /* -> check if mouse is on combo popup */ {
        auto area_begin = bb.Min;
        auto area_end = bb.Max;
        for (int i = 0; i < area_end.y - area_begin.y; i++) {
            for (int n = 0; n < area_end.x - area_begin.x; n++) {
                if (mouse_pos.y == area_begin.y + i && mouse_pos.x == area_begin.x + n) {
                    hovered = true;
                }
            }
        }
    }
    static struct listbox_state {
        animation_vec4 outline{};
    };

    static std::map < ImGuiID, listbox_state > anim;
    auto it_anim = anim.find(id);
    if (it_anim == anim.end())
    {
        anim.insert({ id, {} });
        it_anim = anim.find(id);
    }

    it_anim->second.outline.interpolate(ImColor(28, 28, 28).Value, ImColor(24, 24, 24).Value, hovered);

    if (!IsRectVisible(bb.Min, bb.Max))
    {
        ItemSize(bb.GetSize(), style.FramePadding.y);
        ItemAdd(bb, 0, &frame_bb);
        g.NextWindowData.ClearFlags(); // We behave like Begin() and need to consume those values
        return false;
    }

    PushStyleVar(ImGuiStyleVar_WindowPadding, { c_scale->get(10), c_scale->get(10) });

    BeginGroup();

    PushStyleVar(ImGuiStyleVar_ChildRounding, c_scale->get(3));
    PushStyleColor(ImGuiCol_ChildBg, ImColor(18, 18, 18).Value);
    BeginChild(id, frame_bb.GetSize());
    PopStyleColor();
    PopStyleVar();


    window->DrawList->AddRect(bb.Min, bb.Max, static_cast<ImColor>(c_utils->process_alpha(it_anim->second.outline.value, style.Alpha)), style.FrameRounding);

    return true;
}

void custom_elements_t::end_listbox()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    IM_ASSERT((window->Flags & ImGuiWindowFlags_ChildWindow) && "Mismatched BeginListBox/EndListBox calls. Did you test the return value of BeginListBox?");
    IM_UNUSED(window);

    EndChild();
    EndGroup(); // This is only required to be able to do IsItemXXX query on the whole ListBox including label

    PopStyleVar();
}

bool custom_elements_t::list_box(const char* label, int* current_item, const char* const items[], int items_count, int height_items)
{
    const bool value_changed = list_box(label, current_item, Items_ArrayGetter, (void*)items, items_count, height_items);
    return value_changed;
}

// This is merely a helper around BeginListBox(), EndListBox().
// Considering using those directly to submit custom data or store selection differently.
bool custom_elements_t::list_box(const char* label, int* current_item, const char* (*getter)(void* user_data, int idx), void* user_data, int items_count, int height_in_items)
{
    ImGuiContext& g = *GImGui;

    // Calculate size from "height_in_items"
    if (height_in_items < 0)
        height_in_items = ImMin(items_count, 7);
    float height_in_items_f = height_in_items + 0.25f;
    ImVec2 size(0.0f, ImTrunc(GetTextLineHeightWithSpacing() * height_in_items_f + g.Style.FramePadding.y * 2.0f));

    if (!this->begin_listbox(label, size))
        return false;

    // Assume all items have even height (= 1 line of text). If you need items of different height,
    // you can create a custom version of ListBox() in your code without using the clipper.
    SetCursorPosY(GetCursorPosY() + c_scale->get(10));
    bool value_changed = false;
    ImGuiListClipper clipper;
    clipper.Begin(items_count, GetTextLineHeightWithSpacing()); // We know exactly our line height here so we pass it as a minor optimization, but generally you don't need to.
    clipper.IncludeItemByIndex(*current_item);
    while (clipper.Step())
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
        {

            const char* item_text = getter(user_data, i);
            if (item_text == NULL)
                item_text = "*Unknown item*";

            PushID(i);
            const bool item_selected = (i == *current_item);
            SetCursorPos(GetCursorPos() + ImVec2(c_scale->get(10), 0));
            if (this->selectable(item_text, item_selected))
            {
                *current_item = i;
                value_changed = true;
            }
            if (item_selected)
                SetItemDefaultFocus();
            PopID();
        }
    end_listbox();

    if (value_changed)
        MarkItemEdited(g.LastItemData.ID);

    return value_changed;
}
