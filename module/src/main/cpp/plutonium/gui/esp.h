ImVec2 CalcTextSize(const char* text, ImFont* font, float font_size)
{
    ImVec2 text_size = font->CalcTextSizeA(font_size, FLT_MAX, -1.0f, text, NULL, NULL);

    text_size.x = IM_FLOOR(text_size.x + 0.99999f);

    return text_size;
}

inline void draw_bone(ImDrawList *draw_list, const ImVec2 &pos1, const ImVec2 &pos2, ImColor &color, float thickness)
{
    static ImColor outline_color = ImColor(0, 0, 0);

    draw_list->AddLine(pos1, pos2, color, thickness);
}

void draw_gradient_filled(ImDrawList *draw_list, const ImVec2 &a, const ImVec2 &b, const ImColor color1, const ImColor color2)
{
    draw_list->AddRectFilledMultiColor(a, b, color1, color1, color2, color2);
}

void draw_skeleton(ImDrawList *draw_list, engine::player_t &player_data, menu::cvars &vars, float alpha)
{
    ImColor visible_skeleton_color   = vars.colors.skeleton_visible_color;
    ImColor invisible_skeleton_color = vars.colors.skeleton_invisible_color;
    if (player_data.is_untouchable)
    {
        visible_skeleton_color.Value.w = alpha;
        invisible_skeleton_color.Value.w = alpha;
    }

    static ImColor  outline_color            = ImColor(0, 0, 0);

    const float box_width = (player_data.screen_player_position.y - player_data.screen_player_spine_position.y) * 0.4f;
    engine::hitbox_t &head_box = player_data.hitboxes[engine::bone_type::head];

    float thickness = 2.25f;

    //draw_list->AddCircle(ImVec2(player_data.head_screen_position.x, player_data.head_screen_position.y), box_width * 0.3f, outline_color, 30, thickness + thickness / 2.f);
    draw_list->AddCircle(ImVec2(player_data.head_screen_position.x, player_data.head_screen_position.y), box_width * 0.3f, player_data.head_visible ? visible_skeleton_color : invisible_skeleton_color, 30, thickness);

    draw_bone(draw_list, ImVec2(player_data.neck_screen_position.x, player_data.neck_screen_position.y), ImVec2(player_data.head_screen_position.x, player_data.head_screen_position.y), player_data.neck_visible ? visible_skeleton_color : invisible_skeleton_color, thickness);
    draw_bone(draw_list, ImVec2(player_data.spine2_screen_position.x, player_data.spine2_screen_position.y), ImVec2(player_data.neck_screen_position.x, player_data.neck_screen_position.y), player_data.spine2_visible ? visible_skeleton_color : invisible_skeleton_color, thickness);
    draw_bone(draw_list, ImVec2(player_data.spine1_screen_position.x, player_data.spine1_screen_position.y), ImVec2(player_data.spine2_screen_position.x, player_data.spine2_screen_position.y), player_data.spine1_visible ? visible_skeleton_color : invisible_skeleton_color, thickness);
    draw_bone(draw_list, ImVec2(player_data.hip_screen_position.x, player_data.hip_screen_position.y), ImVec2(player_data.spine1_screen_position.x, player_data.spine1_screen_position.y), player_data.hip_visible ? visible_skeleton_color : invisible_skeleton_color, thickness);
    draw_bone(draw_list, ImVec2(player_data.left_shoulder_screen_position.x, player_data.left_shoulder_screen_position.y), ImVec2(player_data.neck_screen_position.x, player_data.neck_screen_position.y), player_data.left_shoulder_visible ? visible_skeleton_color : invisible_skeleton_color, thickness);
    draw_bone(draw_list, ImVec2(player_data.right_shoulder_screen_position.x, player_data.right_shoulder_screen_position.y), ImVec2(player_data.neck_screen_position.x, player_data.neck_screen_position.y), player_data.right_shoulder_visible ? visible_skeleton_color : invisible_skeleton_color, thickness);
    draw_bone(draw_list, ImVec2(player_data.left_upperarm_screen_position.x, player_data.left_upperarm_screen_position.y), ImVec2(player_data.left_shoulder_screen_position.x, player_data.left_shoulder_screen_position.y), player_data.left_upperarm_visible ? visible_skeleton_color : invisible_skeleton_color, thickness);
    draw_bone(draw_list, ImVec2(player_data.right_upperarm_screen_position.x, player_data.right_upperarm_screen_position.y), ImVec2(player_data.right_shoulder_screen_position.x, player_data.right_shoulder_screen_position.y), player_data.right_upperarm_visible ? visible_skeleton_color : invisible_skeleton_color, thickness);
    draw_bone(draw_list, ImVec2(player_data.left_forearm_screen_position.x, player_data.left_forearm_screen_position.y), ImVec2(player_data.left_upperarm_screen_position.x, player_data.left_upperarm_screen_position.y), player_data.left_forearm_visible ? visible_skeleton_color : invisible_skeleton_color, thickness);
    draw_bone(draw_list, ImVec2(player_data.right_forearm_screen_position.x, player_data.right_forearm_screen_position.y), ImVec2(player_data.right_upperarm_screen_position.x, player_data.right_upperarm_screen_position.y), player_data.right_forearm__visible ? visible_skeleton_color : invisible_skeleton_color, thickness);
    draw_bone(draw_list, ImVec2(player_data.left_hand_screen_position.x, player_data.left_hand_screen_position.y), ImVec2(player_data.left_forearm_screen_position.x, player_data.left_forearm_screen_position.y), player_data.left_hand_visible ? visible_skeleton_color : invisible_skeleton_color, thickness);
    draw_bone(draw_list, ImVec2(player_data.right_hand_screen_position.x, player_data.right_hand_screen_position.y), ImVec2(player_data.right_forearm_screen_position.x, player_data.right_forearm_screen_position.y),player_data.right_hand_visible ? visible_skeleton_color : invisible_skeleton_color, thickness);
    draw_bone(draw_list, ImVec2(player_data.left_thigh_screen_position.x, player_data.left_thigh_screen_position.y), ImVec2(player_data.hip_screen_position.x, player_data.hip_screen_position.y), player_data.left_thigh_visible ? visible_skeleton_color : invisible_skeleton_color, thickness);
    draw_bone(draw_list, ImVec2(player_data.right_thigh_screen_position.x, player_data.right_thigh_screen_position.y), ImVec2(player_data.hip_screen_position.x, player_data.hip_screen_position.y), player_data.right_thigh_visible ? visible_skeleton_color : invisible_skeleton_color, thickness);
    draw_bone(draw_list, ImVec2(player_data.left_calf_screen_position.x, player_data.left_calf_screen_position.y), ImVec2(player_data.left_thigh_screen_position.x, player_data.left_thigh_screen_position.y), player_data.left_calf_visible ? visible_skeleton_color : invisible_skeleton_color, thickness);
    draw_bone(draw_list, ImVec2(player_data.right_calf_screen_position.x, player_data.right_calf_screen_position.y), ImVec2(player_data.right_thigh_screen_position.x, player_data.right_thigh_screen_position.y), player_data.right_calf_visible ? visible_skeleton_color : invisible_skeleton_color, thickness);
    draw_bone(draw_list, ImVec2(player_data.left_foot_screen_position.x, player_data.left_foot_screen_position.y), ImVec2(player_data.left_calf_screen_position.x, player_data.left_calf_screen_position.y), player_data.left_foot_visible ? visible_skeleton_color : invisible_skeleton_color, thickness);
    draw_bone(draw_list, ImVec2(player_data.right_foot_screen_position.x, player_data.right_foot_screen_position.y), ImVec2(player_data.right_calf_screen_position.x, player_data.right_calf_screen_position.y), player_data.right_foot_visible ? visible_skeleton_color : invisible_skeleton_color, thickness);
}

void draw_health(ImDrawList *draw_list, const ImVec2 &top, const ImVec2 &bot, int health, ImColor outline_color, float thickness, float alpha)
{
    int h = std::clamp< int >( health, 0, 100 );
    float box_width = bot.x - top.x;

    float fraction = h * 0.01f;
    ImColor color( std::min( 255 * ( 100 - health ) / 100, 255 ), (int ) (255 * fraction), 0, (int ) (alpha * 255.f) );
    ImColor shadow_color( std::min( 255 * ( 100 - health ) / 100, 255 ), (int ) (255 * fraction), 0, (int ) ((alpha / 2) * 255.f) );

    float height = bot.y - top.y;
    float pos = height - ( height * fraction );

    thickness /= 2;

    draw_list->AddShadowRect(ImVec2(top.x - 5.f - thickness - 0.7f, bot.y + thickness), ImVec2(top.x - 4.f - thickness - 0.7f, top.y + pos - thickness), shadow_color, 15.f, ImVec2(), 0);
    draw_list->AddLine(ImVec2(top.x - 6.f - thickness - 0.7f, top.y - 1.f - thickness), ImVec2(top.x - 6.f - thickness - 0.7f, bot.y + 1.f + thickness), outline_color);
    draw_list->AddLine(ImVec2(top.x - 5.f - thickness - 0.7f, top.y - 1.f - thickness), ImVec2(top.x - 5.f - thickness - 0.7f, bot.y + 1.f + thickness), outline_color);
    draw_list->AddLine(ImVec2(top.x - 4.f - thickness - 0.7f, top.y - 1.f - thickness), ImVec2(top.x - 4.f - thickness - 0.7f, bot.y + 1.f + thickness), outline_color);
    draw_list->AddLine(ImVec2(top.x - 3.f - thickness - 0.7f, top.y - 1.f - thickness), ImVec2(top.x - 3.f - thickness - 0.7f, bot.y + 1.f + thickness), outline_color);

    draw_list->AddLine(ImVec2(top.x - 5.f - thickness - 0.7f, top.y + pos - thickness), ImVec2(top.x - 5.f - thickness - 0.7f, bot.y + thickness), color);
    draw_list->AddLine(ImVec2(top.x - 4.f - thickness - 0.7f, top.y + pos - thickness), ImVec2(top.x - 4.f - thickness - 0.7f, bot.y + thickness), color);

    if ( health != 100 )
    {
        char *health_text = (char *) malloc(64);
        ImFormatString(health_text, 64, "%d", health);

        float font_size = box_width / 8.f;
        ImVec2 text_size = CalcTextSize(health_text, fonts::pixel_font, font_size);

        draw_list->AddText(fonts::pixel_font, font_size + 7.f, ImVec2(top.x - 4.f - text_size.x / 2, top.y + pos - 4.0f + text_size.y / 2), ImColor( 0, 0, 0 ), health_text, NULL, 0);
        draw_list->AddText(fonts::pixel_font, font_size + 7.f, ImVec2(top.x - 5.f - text_size.x / 2, top.y + pos - 5.0f + text_size.y / 2), ImColor( 255, 255, 255 ), health_text, NULL, 0);
    }
}

void draw_esp(plutonium_t *&_cheat, unity::screen &_screen)
{
    auto &vars = menu::vars;
    if (!vars.bools.enable_esp) { return; }

    auto *&_players = _cheat->_players;

    if (_players)
    {
        _players->set_manager();

        for (int i{}; i < _players->count(); i++)
        {
            engine::player_t &player_data = _players->get_enemies()[i];
            if (!player_data.is_valid()) { continue; }

            axlebolt::player_controller *player_controller = player_data.controller;
            axlebolt::object_occludee   *occludee          = player_controller->get_occludee();
            //if (!occludee) { continue; }

            const float box_width           = (player_data.screen_player_position.y - player_data.screen_player_spine_position.y) * 0.4f;
            const float box_height_modifier = box_width * 0.6f;

            const ImRect &rect = ImRect(ImVec2(player_data.screen_player_spine_position.x - box_width, player_data.head_screen_position.y - box_height_modifier),
                                        ImVec2(player_data.screen_player_spine_position.x + box_width, player_data.screen_player_left_foot_position.y + box_height_modifier));

           // ImRect rect = player_data.box;

            auto draw_list = ImGui::GetBackgroundDrawList();

            float old_alpha {};
            if (player_data.is_untouchable)
            {
                old_alpha = menu::vars.colors.box_color.Value.w;
                menu::vars.colors.box_color.Value.w /= 3.f;
                vars.colors.outline_color.Value.w = 0.f;
            }

            if (vars.bools.enable_skeleton)
            {
                draw_skeleton(draw_list, player_data, vars, menu::vars.colors.box_color.Value.w);
            }

            if (vars.bools.enable_box)
            {
                float thickness         = 0.5f + (vars.floats.box_thickness - 1.f) / 10.f;
                float outline_thickness = 0.5f;

                const ImRect &outline_rect_min = ImRect(ImVec2(rect.Min.x + outline_thickness, rect.Min.y + outline_thickness),
                                                        ImVec2(rect.Max.x - outline_thickness, rect.Max.y - outline_thickness));

                const ImRect &outline_rect_max = ImRect(ImVec2(rect.Min.x - outline_thickness, rect.Min.y - outline_thickness),
                                                        ImVec2(rect.Max.x + outline_thickness, rect.Max.y + outline_thickness));

                draw_gradient_filled(draw_list, rect.Min, rect.Max, ImColor(0, 0, 0, 150), ImColor(menu::vars.colors.box_color.Value.x, menu::vars.colors.box_color.Value.y, menu::vars.colors.box_color.Value.z, menu::vars.colors.box_color.Value.w - 0.4f));

                draw_list->AddRect(outline_rect_min.Min, outline_rect_min.Max, vars.colors.outline_color, menu::vars.floats.box_rounding, 0, thickness);
                draw_list->AddRect(outline_rect_max.Min, outline_rect_max.Max, vars.colors.outline_color, menu::vars.floats.box_rounding, 0, thickness);

                draw_list->AddRect(rect.Min, rect.Max, menu::vars.colors.box_color, menu::vars.floats.box_rounding, 0, thickness);
            }

            if (vars.bools.enable_line)
            {
                draw_list->AddLine(ImVec2(_screen.get_width( ) / 2, 0.f), ImVec2(player_data.screen_player_spine_position.x, rect.Min.y), vars.colors.outline_color, 1.f + menu::vars.floats.box_thickness / 5.f + 1.f);
                draw_list->AddLine(ImVec2(_screen.get_width( ) / 2, 0.f), ImVec2(player_data.screen_player_spine_position.x, rect.Min.y), menu::vars.colors.box_color, 1.f + menu::vars.floats.box_thickness / 5.f);
            }

            if (vars.bools.enable_hpbar)
            {
                float thickness = - 1.5f + (vars.floats.box_thickness - 1.f) / 10.f;

                draw_health(draw_list, rect.Min, rect.Max, player_data.health, vars.colors.outline_color, thickness, menu::vars.colors.box_color.Value.w);
            }

            if (vars.bools.enable_name)
            {
                const char *player_nick = player_data.nick;

                ImVec2 text_size     = CalcTextSize(player_nick, fonts::tahoma, fonts::tahoma->FontSize);
                ImVec2 nick_position = ImVec2(player_data.screen_player_spine_position.x - (text_size.x / 2), rect.Min.y - text_size.y - 1.1f - (menu::vars.floats.box_thickness * 0.1f / 2.f));
                ImVec2 back_position = ImVec2(nick_position.x + 1, nick_position.y + 1);

                draw_list->AddText(fonts::tahoma, fonts::tahoma->FontSize, back_position, ImColor(0, 0, 0, (int) (menu::vars.colors.box_color.Value.w * 255.f)), player_nick, NULL, 0);
                draw_list->AddText(fonts::tahoma, fonts::tahoma->FontSize, nick_position, ImColor(255, 255, 255, (int) (menu::vars.colors.box_color.Value.w * 255.f)), player_nick, NULL, 0);
            }

            if (player_data.is_untouchable)
            {
                menu::vars.colors.box_color.Value.w = old_alpha;
                vars.colors.outline_color.Value.w = menu::vars.colors.box_color.Value.w;
            }
        }
    }
}
