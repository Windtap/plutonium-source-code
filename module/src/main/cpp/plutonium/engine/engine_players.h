#include "plutonium/engine/exploits/occlusion.h"

namespace engine
{
#define MAX_PLAYERS_COUNT 32

    plutonium_t::itcu_t *_itcu;
    uintptr_t            il2cpp_addr{};

    enum bone_type // typeDefIndex: 10167
    {
        head = 0,
        neck = 1,
        spine1 = 2,
        spine2 = 3,
        left_upperarm = 4,
        left_forearm = 5,
        left_hand = 6,
        left_shoulder = 7,
        right_shoulder = 8,
        right_upperarm = 9,
        right_forearm = 10,
        right_hand = 11,
        hip = 12,
        left_thigh = 13,
        left_calf = 14,
        left_foot = 15,
        right_thigh = 16,
        right_calf = 17,
        right_foot = 18,
        rightthumb0 = 19,
        leftthumb0 = 20,
    };

    enum collider_type
    {
        box,
        capsule
    };

    struct box_collider
    {
        Vector3 box_center;
        Vector3 size;

        inline Vector3 top()
        {
            float x = box_center.x;
            float y = box_center.y + size.y / 2.f;
            float z = box_center.z;

            return { x, y, z };
        }

        inline Vector3 bottom()
        {
            float x = box_center.x;
            float y = box_center.y - size.y / 2.f;
            float z = box_center.z;

            return { x, y, z };
        }

        inline Vector3 left()
        {
            float x = box_center.x - size.x / 2.f;
            float y = box_center.y;
            float z = box_center.z;

            return { x, y, z };
        }

        inline Vector3 right()
        {
            float x = box_center.x + size.x / 2.f;
            float y = box_center.y;
            float z = box_center.z;

            return { x, y, z };
        }
    };

    struct capsule_collider
    {
        Vector3 capsule_center;
        int direction;
        float radius;
        float height;

        inline Vector3 top()
        {
            float x = capsule_center.x;
            float y = capsule_center.y + radius / 2.f + height / 2.f;
            float z = capsule_center.z;

            return { x, y, z };
        }

        inline Vector3 bottom()
        {
            float x = capsule_center.x;
            float y = capsule_center.y - radius / 2.f - height / 2.f;
            float z = capsule_center.z;

            return { x, y, z };
        }

        inline Vector3 left()
        {
            float x = capsule_center.x - radius / 2.f;
            float y = capsule_center.y;
            float z = capsule_center.z;

            return { x, y, z };
        }

        inline Vector3 right()
        {
            float x = capsule_center.x + radius / 2.f;
            float y = capsule_center.y;
            float z = capsule_center.z;

            return { x, y, z };
        }
    };

    struct hitbox_t
    {
        collider_type type;
        Vector3 position;

        union
        {
            box_collider box;
            capsule_collider capsule;
        };

        inline Vector3 center()
        {
            switch(type)
            {
                case collider_type::box:
                    return box.box_center;
                    break;
                case collider_type::capsule:
                    return capsule.capsule_center;
                    break;
                default:
                    return {};
            }
        }

        inline Vector3 top()
        {
            switch(type)
            {
                case collider_type::box:
                    return box.top();
                    break;
                case collider_type::capsule:
                    return capsule.top();
                    break;
                default:
                    return {};
            }
        }

        inline Vector3 bottom()
        {
            switch(type)
            {
                case collider_type::box:
                    return box.bottom();
                    break;
                case collider_type::capsule:
                    return capsule.bottom();
                    break;
                default:
                    return {};
            }
        }

        inline Vector3 left()
        {
            switch(type)
            {
                case collider_type::box:
                    return box.left();
                    break;
                case collider_type::capsule:
                    return capsule.left();
                    break;
                default:
                    return {};
            }
        }

        inline Vector3 right()
        {
            switch(type)
            {
                case collider_type::box:
                    return box.right();
                    break;
                case collider_type::capsule:
                    return capsule.right();
                    break;
                default:
                    return {};
            }
        }

        Vector3 vectors_enum(int vi)
        {
            switch (vi)
            {
                case 0:
                {
                    return top();
                }
                break;

                case 1:
                {
                    return bottom();
                }
                break;

                case 2:
                {
                    return left();
                }
                    break;

                case 3:
                {
                    return right();
                }
                break;

                case 4:
                {
                    return center();
                }
                    break;

                default:
                    return {};
            }
        }

        inline Vector3 to_front(Vector3 point)
        {
            switch(type)
            {
                case collider_type::box:
                    return point + Vector3(0.f, 0.f, box.size.z / 2.f);
                    break;
                case collider_type::capsule:
                    return point + Vector3(0.f, 0.f, capsule.radius / 2.f);
                    break;
                default:
                    return {};
            }
        }

        inline Vector3 to_half_front(Vector3 point)
        {
            switch(type)
            {
                case collider_type::box:
                    return point + Vector3(0.f, 0.f, box.size.z / 4.f);
                    break;
                case collider_type::capsule:
                    return point + Vector3(0.f, 0.f, capsule.radius / 4.f);
                    break;
                default:
                    return {};
            }
        }
    };

    struct bone_t
    {
        bone_type bone;
        Vector3 screen_position;
        Vector3 position;

        bool visible;
    };

    struct player_t
    {
        int photon_id;

        bool is_local;
        int  health;
        int  armor;

        const char *nick;

        int hitbox_count;
        hitbox_t *hitboxes;

        uint8_t team;
        ImRect box;

        Vector3 screen_player_position;
        Vector3 screen_player_left_foot_position;
        Vector3 screen_player_spine_position;

        Vector3 neck_screen_position;
        Vector3 head_screen_position;
        Vector3 spine1_screen_position;
        Vector3 spine2_screen_position;
        Vector3 hip_screen_position;
        Vector3 left_shoulder_screen_position;
        Vector3 right_shoulder_screen_position;
        Vector3 left_upperarm_screen_position;
        Vector3 right_upperarm_screen_position;
        Vector3 left_forearm_screen_position;
        Vector3 right_forearm_screen_position;
        Vector3 left_hand_screen_position;
        Vector3 right_hand_screen_position;
        Vector3 left_thigh_screen_position;
        Vector3 right_thigh_screen_position;
        Vector3 left_calf_screen_position;
        Vector3 right_calf_screen_position;
        Vector3 left_foot_screen_position;
        Vector3 right_foot_screen_position;

        bool head_visible;
        bool neck_visible;
        bool spine1_visible;
        bool spine2_visible;
        bool hip_visible;
        bool left_shoulder_visible;
        bool right_shoulder_visible;
        bool left_upperarm_visible;
        bool right_upperarm_visible;
        bool left_forearm_visible;
        bool right_forearm__visible;
        bool left_hand_visible;
        bool right_hand_visible;
        bool left_thigh_visible;
        bool right_thigh_visible;
        bool left_calf_visible;
        bool right_calf_visible;
        bool left_foot_visible;
        bool right_foot_visible;

        axlebolt::player_controller *controller;
        bool is_untouchable;

        bool valid;

        bool is_valid()
        {
            return valid && controller && health > 0;
        }
    };

    class players
    {
    private:
        static    players         *instance;
        axlebolt::player_manager  *manager;
        axlebolt::player_controls *controls;

        player_t  local;
        player_t *enemies;

        size_t enemies_count;

    public:
        void set_manager()
        {
            static il2cpp::klass *player_manager_klass                 = _itcu->find_class(nullptr, "PlayerManager");
            static il2cpp::klass *lazy_singleton__player_manager_class = (il2cpp::klass *) player_manager_klass->_1.parent;

            static void *static_fields = lazy_singleton__player_manager_class->static_fields;

            manager = * (axlebolt::player_manager **) static_fields;
        }

        player_t *&get_enemies()
        {
            return enemies;
        }

        axlebolt::player_manager *get_manager()
        {
            if (manager)
            {
                return manager;
            }

            return nullptr;
        }

        axlebolt::player_controls *get_controls()
        {
            static il2cpp::klass *controls_klass       = _itcu->find_class(nullptr, "PlayerControls");
            static void          *static_fields        = controls_klass->static_fields;

            auto player_controls_instance = * (axlebolt::player_controls **) static_fields;
            //this->controls = player_controls_instance;

            return player_controls_instance;
        }

        axlebolt::player_controller *get_local()
        {
            auto player_controls_instance = this->get_controls();
            return player_controls_instance->player;
        }

        int count()
        {
            if (manager)
            {
                return manager->_playerById->count;
            }

            return NULL;
        }

        static players *&get()
        {
            return instance;
        }

        static players *create()
        {
            instance          = _g_cheat.load()->_allocator->calloc<players>();
            instance->enemies = _g_cheat.load()->_allocator->aalloc<player_t>(MAX_PLAYERS_COUNT);

            return instance;
        }
    };

#define INIT_DATA(data,name) data.name = name

    players *players::instance = nullptr;

    void player_controller__update(axlebolt::player_controller *_this)
    {
        static plutonium_t *_cheat   = _g_cheat.load();
        static players     *_players = _cheat->_players;

        axlebolt::player_controller::orig_update(_this);

        //if (!menu::vars.bools.enable_esp) { return; }
        static unity::screen screen = unity::screen::get_screen();
        unity::camera       *camera = unity::camera::get_main();

        if (!camera) { return; }
        //set_ignore_raycasting();

        axlebolt::player_controls *controls_instance = _players->get_controls();
        unity::transform          *camera_transform  = camera->get_component()->get_transform();
        Vector3                    camera_position   = camera_transform->get_position();
        Quaternion                 camera_rotation   = camera_transform->get_rotation();
        float                      fov               = camera->get_fieldOfView();

        axlebolt::player_controller *local = _players->get_local();
        if (_this && local && _this == local)
        {
            _players->set_manager();
            il2cpp::dictionary<int, axlebolt::player_controller *> *players_dictionary = _players->get_manager()->_playerById;
            player_t *&enemies = _players->get_enemies();

            int j{};
            for (int i{}; i < players_dictionary->count && j < MAX_PLAYERS_COUNT; i++)
            {
                auto *&player = players_dictionary->entries->items[i].value;

                if (player)
                {
                    axlebolt::photon_player *photon_player = player->photon_player;
                    player_t &player_data  = enemies[j]; j++;

                    if (local->get_team() != player->get_team() && photon_player)
                    {
                        player_data.controller = player;

                        unity::transform          *player_transform = player->get_component()->get_transform();
                        const axlebolt::biped_map *player_biped     = player->get_biped_map();

                        player_data.valid      = true;
                        player_data.controller = player;

                        if (!player_transform || !player_biped || !player_biped->Head || !player_biped->Neck)
                        {
                            player_data.valid      = false;
                            player_data.controller = nullptr;
                            j--;
                            continue;
                        }

                        const Vector3 player_head_position      = player_biped->Spine1->get_position();
                        const Vector3 player_left_foot_position = player_biped->LeftFoot->get_position();
                        const Vector3 player_spine_position     = player_biped->Spine2->get_position();
                        const Vector3 player_position           = player_transform->get_position();

                        /*unity::bounds bounds = player->get_collider()->get_bounds();
                        Vector3       min    = world_to_screen(bounds.min(), camera_position, camera_rotation, fov);
                        Vector3       max    = world_to_screen(bounds.max(), camera_position, camera_rotation, fov);

                        ImRect box = ImRect(ImVec2(max.x, max.y),
                                            ImVec2(min.x, min.y));

                        INIT_DATA(player_data, box);*/

                        const Vector3 screen_player_position           = world_to_screen(player_position, camera_position, camera_rotation, fov);
                        const Vector3 screen_player_head_position      = world_to_screen(player_head_position, camera_position, camera_rotation, fov);
                        const Vector3 screen_player_spine_position     = world_to_screen(player_spine_position, camera_position, camera_rotation, fov);
                        const Vector3 screen_player_left_foot_position = world_to_screen(player_left_foot_position, camera_position, camera_rotation, fov);

                        /*if (menu::vars.bools.enable_skeleton)
                        {
                            unity::skinned_mesh_renderer *renderer = player->get_renderer();

                            if (renderer)
                            {
                                unity::mesh *mesh = renderer->get_shared_mesh();
                                unity::bounds bounds = mesh->get_bounds();
                                Vector3       min    = world_to_screen(bounds.min() + player_position, camera_position, camera_rotation, fov);
                                Vector3       max    = world_to_screen(bounds.max() + player_position, camera_position, camera_rotation, fov);

                                ImRect box = ImRect(ImVec2(max.x, max.y),
                                                    ImVec2(min.x, min.y));

                                INIT_DATA(player_data, box);
                            }
                        }*/

                        if (screen_player_position.z <= 0.50f || screen_player_head_position.z <= 0.50f)
                        {
                            player_data.valid      = false;
                            player_data.controller = nullptr;
                            j--;
                            continue;
                        }

                        auto hitboxes = player->get_bones();
                        constexpr int hitbox_count = 20;
                        player_data.hitbox_count = hitbox_count;

                        hitbox_t *&player_hitboxes = player_data.hitboxes;

                        if (!player_hitboxes)
                        {
                            player_hitboxes = _cheat->_allocator->aalloc<hitbox_t>(hitbox_count);
                            player_data.hitboxes = player_hitboxes;
                        }

                        for (int k{}; k < hitbox_count; k++)
                        {
                            axlebolt::player_hitbox *phitbox = hitboxes->get_value(k);
                            if (!phitbox) { continue; }

                            unity::transform *bone_transform = player->get_bone(phitbox->config->bone);

                            hitbox_t &hitbox = player_hitboxes[k];
                            axlebolt::hitbox_config *phitbox_config = phitbox->config;

                            if (phitbox_config->type == axlebolt::box)
                            {
                                hitbox.type = box;
                                hitbox.position = bone_transform->get_position();

                                Vector3 center = phitbox_config->center;
                                Vector3 size   = phitbox_config->size;

                                hitbox.box.size = size;
                                hitbox.box.box_center = center + hitbox.position;
                            }
                            else if (phitbox_config->type == axlebolt::capsule || (int) phitbox_config->type == 3)
                            {
                                hitbox.type = capsule;
                                hitbox.position = bone_transform->get_position();
                                hitbox.capsule.capsule_center = bone_transform->transform_point(phitbox_config->center);

                                hitbox.capsule.radius = phitbox_config->radius;
                                hitbox.capsule.height = phitbox_config->height;
                                hitbox.capsule.direction = phitbox_config->direction;
                            }
                        }

                        player_data.health         = photon_player->get_health();
                        player_data.is_untouchable = photon_player->is_untouchable();

                        INIT_DATA(player_data, screen_player_left_foot_position);
                        INIT_DATA(player_data, screen_player_position);
                        INIT_DATA(player_data, screen_player_spine_position);

                        hitbox_t &head_box = player_hitboxes[bone_type::head];

                        const Vector3 head_screen_position = world_to_screen(head_box.to_half_front(head_box.capsule.capsule_center + Vector3(0.f, head_box.capsule.height / 4.f, 0)), camera_position, camera_rotation, fov);
                        INIT_DATA(player_data, head_screen_position);

                        const char *nick   = photon_player->get_nick();
                        INIT_DATA(player_data, nick);

                        constexpr static int bullet_layer = 16384;

                        if (menu::vars.bools.enable_firecheck && player_data.is_valid())
                        {
                            chams::set_player(player_data.controller);
                        }

                        const Vector3 neck_screen_position           = world_to_screen(player_biped->Neck->get_position(), camera_position, camera_rotation, fov);
                        const Vector3 spine1_screen_position         = world_to_screen(player_biped->Spine1->get_position(), camera_position, camera_rotation, fov);
                        const Vector3 spine2_screen_position         = world_to_screen(player_biped->Spine2->get_position(), camera_position, camera_rotation, fov);
                        const Vector3 hip_screen_position            = world_to_screen(player_biped->Hip->get_position(), camera_position, camera_rotation, fov);
                        const Vector3 left_shoulder_screen_position  = world_to_screen(player_biped->LeftShoulder->get_position(), camera_position, camera_rotation, fov);
                        const Vector3 right_shoulder_screen_position = world_to_screen(player_biped->RightShoulder->get_position(), camera_position, camera_rotation, fov);
                        const Vector3 left_upperarm_screen_position  = world_to_screen(player_biped->LeftUpperarm->get_position(), camera_position, camera_rotation, fov);
                        const Vector3 right_upperarm_screen_position = world_to_screen(player_biped->RightUpperarm->get_position(), camera_position, camera_rotation, fov);
                        const Vector3 left_forearm_screen_position   = world_to_screen(player_biped->LeftForearm->get_position(), camera_position, camera_rotation, fov);
                        const Vector3 right_forearm_screen_position  = world_to_screen(player_biped->RightForearm->get_position(), camera_position, camera_rotation, fov);
                        const Vector3 left_hand_screen_position      = world_to_screen(player_biped->LeftHand->get_position(), camera_position, camera_rotation, fov);
                        const Vector3 right_hand_screen_position     = world_to_screen(player_biped->RightHand->get_position(), camera_position, camera_rotation, fov);
                        const Vector3 left_thigh_screen_position     = world_to_screen(player_biped->LeftThigh->get_position(), camera_position, camera_rotation, fov);
                        const Vector3 right_thigh_screen_position    = world_to_screen(player_biped->RightThigh->get_position(), camera_position, camera_rotation, fov);
                        const Vector3 left_calf_screen_position      = world_to_screen(player_biped->LeftCalf->get_position(), camera_position, camera_rotation, fov);
                        const Vector3 right_calf_screen_position     = world_to_screen(player_biped->RightCalf->get_position(), camera_position, camera_rotation, fov);
                        const Vector3 left_foot_screen_position      = world_to_screen(player_biped->LeftFoot->get_position(), camera_position, camera_rotation, fov);
                        const Vector3 right_foot_screen_position     = world_to_screen(player_biped->RightFoot->get_position(), camera_position, camera_rotation, fov);

                        INIT_DATA(player_data, neck_screen_position);
                        INIT_DATA(player_data, spine1_screen_position);
                        INIT_DATA(player_data, spine2_screen_position);
                        INIT_DATA(player_data, hip_screen_position);
                        INIT_DATA(player_data, left_shoulder_screen_position);
                        INIT_DATA(player_data, right_shoulder_screen_position);
                        INIT_DATA(player_data, left_upperarm_screen_position);
                        INIT_DATA(player_data, right_upperarm_screen_position);
                        INIT_DATA(player_data, left_forearm_screen_position);
                        INIT_DATA(player_data, right_forearm_screen_position);
                        INIT_DATA(player_data, left_hand_screen_position);
                        INIT_DATA(player_data, right_hand_screen_position);
                        INIT_DATA(player_data, left_thigh_screen_position);
                        INIT_DATA(player_data, right_thigh_screen_position);
                        INIT_DATA(player_data, left_calf_screen_position);
                        INIT_DATA(player_data, right_calf_screen_position);
                        INIT_DATA(player_data, left_foot_screen_position);
                        INIT_DATA(player_data, right_foot_screen_position);

                        const bool head_visible           = !unity::physics::linecast(camera_position, head_box.to_front(head_box.capsule.capsule_center + Vector3(0.f, head_box.capsule.height / 4.f, 0)), bullet_layer);
                        const bool neck_visible           = !unity::physics::linecast(camera_position, hitboxes->get_value(axlebolt::neck)->get_collider()->get_bounds().center, bullet_layer);
                        const bool spine1_visible         = !unity::physics::linecast(camera_position, player_biped->Spine1->get_position(), bullet_layer);
                        const bool spine2_visible         = !unity::physics::linecast(camera_position, player_biped->Spine2->get_position(), bullet_layer);
                        const bool hip_visible            = !unity::physics::linecast(camera_position, player_biped->Hip->get_position(), bullet_layer);
                        const bool left_shoulder_visible  = !unity::physics::linecast(camera_position, player_biped->LeftShoulder->get_position(), bullet_layer);
                        const bool right_shoulder_visible = !unity::physics::linecast(camera_position, player_biped->RightShoulder->get_position(), bullet_layer);
                        const bool left_upperarm_visible  = !unity::physics::linecast(camera_position, player_biped->LeftUpperarm->get_position(), bullet_layer);
                        const bool right_upperarm_visible = !unity::physics::linecast(camera_position, player_biped->RightUpperarm->get_position(), bullet_layer);
                        const bool left_forearm_visible   = !unity::physics::linecast(camera_position, player_biped->LeftForearm->get_position(), bullet_layer);
                        const bool right_forearm__visible = !unity::physics::linecast(camera_position, player_biped->RightForearm->get_position(), bullet_layer);
                        const bool left_hand_visible      = !unity::physics::linecast(camera_position, player_biped->LeftHand->get_position(), bullet_layer);
                        const bool right_hand_visible     = !unity::physics::linecast(camera_position, player_biped->RightHand->get_position(), bullet_layer);
                        const bool left_thigh_visible     = !unity::physics::linecast(camera_position, player_biped->LeftThigh->get_position(), bullet_layer);
                        const bool right_thigh_visible    = !unity::physics::linecast(camera_position, player_biped->RightThigh->get_position(), bullet_layer);
                        const bool left_calf_visible      = !unity::physics::linecast(camera_position, player_biped->LeftCalf->get_position(), bullet_layer);
                        const bool right_calf_visible     = !unity::physics::linecast(camera_position, player_biped->RightCalf->get_position(), bullet_layer);
                        const bool left_foot_visible      = !unity::physics::linecast(camera_position, player_biped->LeftFoot->get_position(), bullet_layer);
                        const bool right_foot_visible     = !unity::physics::linecast(camera_position, player_biped->RightFoot->get_position(), bullet_layer);

                        INIT_DATA(player_data, head_visible);
                        INIT_DATA(player_data, neck_visible);
                        INIT_DATA(player_data, spine1_visible);
                        INIT_DATA(player_data, spine2_visible);
                        INIT_DATA(player_data, hip_visible);
                        INIT_DATA(player_data, left_shoulder_visible);
                        INIT_DATA(player_data, right_shoulder_visible);
                        INIT_DATA(player_data, left_upperarm_visible);
                        INIT_DATA(player_data, right_upperarm_visible);
                        INIT_DATA(player_data, left_forearm_visible);
                        INIT_DATA(player_data, right_forearm__visible);
                        INIT_DATA(player_data, left_hand_visible);
                        INIT_DATA(player_data, right_hand_visible);
                        INIT_DATA(player_data, left_thigh_visible);
                        INIT_DATA(player_data, right_thigh_visible);
                        INIT_DATA(player_data, left_calf_visible);
                        INIT_DATA(player_data, right_calf_visible);
                        INIT_DATA(player_data, left_foot_visible);
                        INIT_DATA(player_data, right_foot_visible);
                    }
                    else
                    {
                        player_data.valid = false;
                        j--;
                    }
                }
            }

           for (; j < MAX_PLAYERS_COUNT; j++)
           {
               enemies[j].valid = false;
           }
        }
    }
}