namespace engine
{
    class aimbot
    {
    private:
        static aimbot *instance;
        plutonium_t   *_cheat;

    public:
        enum bone_type
        {
            head,
            neck,
            stomach,
            hands,
            legs,
            foots
        };

        static aimbot *create()
        {
            instance          = _g_cheat.load()->_allocator->calloc<aimbot>();
            instance->_cheat  = _g_cheat.load();

            return instance;
        }

        float get_camera_angle(const Vector3 screen_position)
        {
            if (screen_position.z <= 0.5f) { return 999.f; }

            static unity::screen &screen = unity::screen::get_screen();
            static float width  = (float) screen.get_width();
            static float height = (float) screen.get_height();

            float x_angle = (width / 2 - screen_position.x) * (width / 2 - screen_position.x);
            float y_angle = (height / 2 - screen_position.y) * (height / 2 - screen_position.y);

            float angle = sqrt(x_angle + y_angle);
            return angle;
        }

        bool in_rect(Vector3 &player_screen, float fovx, float fovy)
        {
            if (player_screen.z <= 0.5f) { return false; }
            static auto &screen = unity::screen::get_screen();

            int fovLeft = (screen.get_width() - fovx) / 2;
            int fovTop = (screen.get_height() - fovy) / 2;
            int fovRight = fovLeft + fovx;
            int fovBottom = fovTop + fovy;

            return player_screen.x >= fovLeft && player_screen.x <= fovRight &&
                    player_screen.y >= fovTop && player_screen.y <= fovBottom;
        }

        float distance_to(Vector3 player_screen)
        {
            if (player_screen.z <= 0.5f) { return 999999999.f; }
            static auto &screen = unity::screen::get_screen();

            return Vector2::Distance(Vector2(screen.get_width() / 2, screen.get_height() / 2), Vector2(player_screen.x, player_screen.y));
        }

        float distance_to_player(Vector3 position, Vector3 camera_position, Quaternion camera_rotation, float field_of_view)
        {
            Vector3 player_screen = world_to_screen(position, camera_position, camera_rotation, field_of_view);

            if (player_screen.z <= 0.5f) { return 99999999.f; }
            static auto &screen = unity::screen::get_screen();

            return distance_to(player_screen);
        }

        static inline bool check_bone(axlebolt::bone_type &bone)
        {
            static constexpr int bones_count{6};

            const auto &vars          = menu::vars;
            const bool *enabled_bones = vars.bools.enable_bone;

            if (!enabled_bones) { return false; }

            const bool &enabled_head    = enabled_bones[aimbot::head];
            const bool &enabled_neck    = enabled_bones[aimbot::neck];
            const bool &enabled_stomach = enabled_bones[aimbot::stomach];
            const bool &enabled_hands   = enabled_bones[aimbot::hands];
            const bool &enabled_legs    = enabled_bones[aimbot::legs];
            const bool &enabled_foots   = enabled_bones[aimbot::foots];

            if (enabled_head && bone == axlebolt::head)
            {
                return true;
            }
            else if (enabled_neck && bone == axlebolt::neck)
            {
                return true;
            }
            else if (enabled_stomach && (bone == axlebolt::spine1 || bone == axlebolt::spine2 || bone == axlebolt::hip))
            {
                return true;
            }
            else if (enabled_hands && bone >= axlebolt::leftupperarm && bone <= axlebolt::righthand)
            {
                return true;
            }
            else if (enabled_legs && bone != axlebolt::leftfoot && bone >= axlebolt::leftthigh && bone <= axlebolt::rightcalf)
            {
                return true;
            }
            else if (enabled_foots && (bone == axlebolt::leftfoot || bone == axlebolt::rightfoot))
            {
                return true;
            }

            return false;
        }

        bone_t get_visible_bone(player_t &player_data, menu::cvars &vars, Vector3 &camera_position, Quaternion &camera_rotation, float &field_of_view)
        {
            static constexpr int bone_count{20};

            auto player = player_data.controller;
            float min_distance = 9999999.f;
            bone_t result_bone{.visible = false};

            if (player)
            {
                for (axlebolt::bone_type i{}; i < bone_count; (* (int *) &i)++)
                {
                    if (!check_bone(i)) { continue; }

                    unity::transform *hit_box = player->get_bone(i);

                    if (hit_box)
                    {
                        Vector3 position;
                        engine::hitbox_t &hitbox = player_data.hitboxes[i];
                        constexpr int vector_count{5};
                        bool checked{false};

                        for (int k{}; k < vector_count; k++)
                        {
                            position = hitbox.vectors_enum(k);
                            Vector3 screen_bounds_center = world_to_screen(position, camera_position, camera_rotation, field_of_view);

                            bool  in_fov{};
                            float aim_fov  = vars.floats.aimbot_fov;
                            int   fov_type = vars.integers.fov_type;
                            int   filter   = vars.integers.aiming_type;

                            if (fov_type == 0)
                            {
                                float angle = get_camera_angle(screen_bounds_center);
                                in_fov = angle <= aim_fov;
                            }
                            else if (fov_type == 1)
                            {
                                in_fov = in_rect(screen_bounds_center, aim_fov, vars.floats.aimbot_fovy);
                            }

                            if (in_fov)
                            {
                                constexpr int bullet_layer = 16384;
                                bool is_center_visible = !unity::physics::linecast(camera_position, hitbox.to_front(position), bullet_layer);
                                if (is_center_visible)
                                {
                                    if (filter == 1)
                                    {
                                        if (!checked)
                                        {
                                            checked = true;

                                            float distance = distance_to(screen_bounds_center);
                                            if (distance < min_distance)
                                            {
                                                min_distance = distance;
                                            }
                                            else
                                            {
                                                continue;
                                            }
                                        }
                                    }

                                    const bone_t bone = {
                                            .position = position,
                                            .visible = is_center_visible
                                    };

                                    result_bone = bone;

                                    if (filter == 0)
                                    {
                                        return result_bone;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            return result_bone;
        }

        bone_t get_filtered_bone(player_t *&enemies, menu::cvars &vars, Vector3 &camera_position, Quaternion &camera_rotation, float &field_of_view)
        {
            bone_t filtered_bone{.visible = false};
            float min_distance = 99999999.f;

            for (int i{}; i < MAX_PLAYERS_COUNT; i++)
            {
                player_t &enemy = enemies[i];
                if (!enemy.is_valid()) { continue; }
                if (enemy.is_untouchable && vars.bools.enable_untouchable_check) { continue; }

                const bone_t bone = this->get_visible_bone(enemy, vars, camera_position, camera_rotation, field_of_view);
                if (!bone.visible) { continue; }

                float distance = this->distance_to_player(bone.position, camera_position, camera_rotation, field_of_view);
                if (distance < min_distance)
                {
                    min_distance = distance;
                    filtered_bone = bone;
                }
            }

            return filtered_bone;
        }
    };

    aimbot *aimbot::instance{};

    void gun_controller__execute_commands(axlebolt::gun_controller *_this, axlebolt::weapon_command cmd, float duration, float time)
    {
        static plutonium_t *_cheat   = _g_cheat.load();
        static players     *_players = _cheat->_players;
        static aimbot      *_aimbot  = _cheat->_aimbot;
        static menu::cvars &vars     = menu::cvars::vars();

        if (_this && vars.bools.enable_aimbot && !vars.bools.enable_silent && (cmd.to_fire || !vars.bools.enable_firecheck))
        {
            unity::camera *camera = unity::camera::get_main();
            unity::transform          *camera_transform  = camera->get_component()->get_transform();
            Vector3                    camera_position   = camera_transform->get_position();
            Quaternion                 camera_rotation   = camera_transform->get_rotation();
            float                      fov               = camera->get_fieldOfView();

            axlebolt::player_controller *local = _this->player;
            if (local)
            {
                player_t *&enemies = _players->get_enemies();
                if (enemies)
                {
                    const bone_t &bone = _aimbot->get_filtered_bone(enemies, vars, camera_position, camera_rotation, fov);
                    if (bone.visible)
                    {
                        float t = 1.f - (vars.floats.aimbot_smooth / 100.f);
                        Quaternion lookRotation = Quaternion::LookRotation((bone.position - camera_position));

                        Vector3 eulerAnglesRotation{};
                        eulerAnglesRotation.x = local->get_aiming_data()->current_aim_angle.x;
                        eulerAnglesRotation.y = local->get_aiming_data()->current_euler_angle.y;

                        Quaternion rotation = Quaternion::Slerp(FromEuler(eulerAnglesRotation * 0.017453292f), lookRotation, t * (unity::time::get_deltaTime() / (0.167f / 4)));
                        Vector3 eulerAngles = ToEulerRad(rotation);

                        local->get_aiming_data()->current_aim_angle.x = eulerAngles.x;
                        local->get_aiming_data()->current_euler_angle.y = eulerAngles.y;
                    }
                }
            }
        }

        axlebolt::gun_controller::execute_commands(_this, cmd, duration, time);
    }

    bool (*orig_raycast)(void *scene, Ray *ray, float distance, RaycastHit *hit, int layer, int query);
    bool hk_raycast(void *scene, Ray *ray, float distance, RaycastHit *hit, int layer, int query)
    {
        static plutonium_t *_cheat   = _g_cheat.load();
        static players     *_players = _cheat->_players;
        static aimbot      *_aimbot  = _cheat->_aimbot;
        static menu::cvars &vars     = menu::cvars::vars();

        if (vars.bools.enable_aimbot && vars.bools.enable_silent)
        {
            constexpr int   hit_layer         = 1610637328;
            constexpr float required_distance = 1000.f;

            if (layer == hit_layer && distance == required_distance)
            {
                unity::camera *camera = unity::camera::get_main();

                unity::transform *camera_transform  = camera->get_component()->get_transform();
                Vector3           camera_position   = camera_transform->get_position();
                Quaternion        camera_rotation   = camera_transform->get_rotation();
                float             fov               = camera->get_fieldOfView();

                player_t *&enemies = _players->get_enemies();
                if (enemies)
                {
                    const bone_t &bone = _aimbot->get_filtered_bone(enemies, vars, camera_position, camera_rotation, fov);
                    if (bone.visible)
                    {
                        Vector3 direction = (bone.position - camera_position).normalized();

                        ray->m_Direction = direction;
                    }
                }
            }
        }

        return orig_raycast(scene, ray, distance, hit, layer, query);
    }
}