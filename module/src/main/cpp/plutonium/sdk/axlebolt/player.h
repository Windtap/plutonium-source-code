enum team
{
    none = 0,
    tr = 1,
    ct = 2,
    spect = 3
};

enum bone_type // typeDefIndex: 10167
{
    head = 0,
    neck = 1,
    spine1 = 2,
    spine2 = 3,
    leftupperarm = 4,
    leftforearm = 5,
    lefthand = 6,
    leftshoulder = 7,
    rightshoulder = 8,
    rightupperarm = 9,
    rightforearm = 10,
    righthand = 11,
    hip = 12,
    leftthigh = 13,
    leftcalf = 14,
    leftfoot = 15,
    rightthigh = 16,
    rightcalf = 17,
    rightfoot = 18,
    rightthumb0 = 19,
    leftthumb0 = 20,
};


struct biped_map
{
    il2cpp::klass *klass;
    void          *monitor;

    void *cachedPtr;
    void *cancel_token;

    unity::transform *Head;
    unity::transform *Neck;
    unity::transform *Spine;
    unity::transform *Spine1;
    unity::transform *Spine2;
    unity::transform *LeftShoulder;
    unity::transform *LeftUpperarm;
    unity::transform *LeftForearm;
    unity::transform *LeftHand;
    unity::transform *RightShoulder;
    unity::transform *RightUpperarm;
    unity::transform *RightForearm;
    unity::transform *RightHand;
    unity::transform *Hip;
    unity::transform *LeftThigh;
    unity::transform *LeftCalf;
    unity::transform *LeftFoot;
    unity::transform *LeftToeBase;
    unity::transform *RightThigh;
    unity::transform *RightCalf;
    unity::transform *RightFoot;
    unity::transform *RightThumb0;
    unity::transform *LeftThumb0;
    unity::transform *RFinger0;
    unity::transform *RFinger1;
    unity::transform *RFinger2;
    unity::transform *RFinger3;
    unity::transform *RFinger4;
};

enum hitbox_type
{
    box = 0,
    capsule = 1
};

struct hitbox_config
{
    il2cpp::klass *klass;
    void *monitor;

    alignas(4) bone_type bone;
    alignas(4) hitbox_type type;

    Vector3 center;
    Vector3 size;

    int direction;
    float radius;
    float height;
};

struct player_hitbox
{
    il2cpp::klass *klass;
    void *monitor;

    void *f1;
    void *f2;

    hitbox_config *config;

    unity::collider *get_collider()
    {
        return * (unity::collider **) ((uintptr_t) this + 0x28);
    }
};

struct aiming_data
{
    il2cpp::klass *klass;
    void          *monitor;

    float curWTCoeff;
    float curMTCoeff;

    Vector3 current_aim_angle;
    Vector3 current_euler_angle;
};

struct player_controller
{
    il2cpp::klass *klass;
    void          *monitor;

    uintptr_t    m_cached;
    void        *ct_source;
    photon_view *pv_cache;

    unity::transform   *main_camera_holder;
    unity::game_object *fps_camera_holder;
    unity::game_object *fps_directive;

    uintptr_t zones;
    uintptr_t character_view;
    uintptr_t player_character_view;
    bool      reconnected;
    team      team;
    uintptr_t f1;
    float     local_time;
    uintptr_t aim_controller;
    uintptr_t weaponry_controller;
    uintptr_t mecanim_controller;
    uintptr_t movement_controller;
    uintptr_t arms_animation_controller;
    uintptr_t player_hit_controller;
    uintptr_t player_occlusion_controller;
    uintptr_t network_controller;
    uintptr_t arms_lodgroup;
    uintptr_t player_character_view_controller;
    bool      is_visible;
    bool      is_visible2;
    float     set_snapshot_time;
    uintptr_t player_sound_controller;
    uintptr_t player_main_camera;
    uintptr_t player_fps_camera;
    uintptr_t player_marker_trigger;

    unity::transform *player_transform;

    uintptr_t controllers;
    uintptr_t controllers_dict;
    uintptr_t skinned_mesh;
    uintptr_t character_controller;
    bool      is_preinitialized;
    bool      is_postinitialized;
    int       view_mode;

    unity::nullable<bool> predicted_armor;
    unity::nullable<bool> predicted_health;

    uintptr_t photon_view;
    int       player_id;
    int       player_view_id;

    photon_player *photon_player;

    inline unity::transform *get_bone(int bone)
    {
        bone += 1;

        auto *biped_map = get_biped_map();
        void *bone_ptr  = (void *) ((uintptr_t) biped_map + 0x20 + sizeof(void *) * bone);

        if (biped_map && bone_ptr)
        {
            return * (unity::transform  **) bone_ptr;
        }

        return nullptr;
    }

    inline auto *get_component()
    {
        return (unity::component *) this;
    }

    inline uint8_t get_team()
    {
        return * (uint8_t *) ((uintptr_t) this + 0x59);
    }

    inline biped_map *get_biped_map()
    {
        uintptr_t player_view = * (uintptr_t *) ((uintptr_t) this + 0xA8);
        if (!player_view) { return nullptr; }

        return * (biped_map **) (player_view + 0x48);
    }

    inline auto *get_occludee()
    {
        return * (object_occludee **) ((uintptr_t) this + 0x90);
    }

    unity::skinned_mesh_renderer *get_renderer()
    {
        uintptr_t lod_group = * reinterpret_cast<uintptr_t *>((uintptr_t) this + 0x100);
        if (lod_group)
        {
            auto *meshRenderer = * reinterpret_cast<unity::skinned_mesh_renderer **>(lod_group + 0x38);
            return meshRenderer;
        }

        return {};
    }

    inline auto *get_collider()
    {
        uintptr_t _player_hit_controller = * (uintptr_t *) ((uintptr_t) this + 0x88);
        return * (unity::character_controller **) ((uintptr_t) _player_hit_controller + 0x98);
    }

    inline auto *get_bones()
    {
        uintptr_t _player_hit_controller = * (uintptr_t *) ((uintptr_t) this + 0x88);
        return * (il2cpp::dictionary<int, player_hitbox *> **) ((uintptr_t) _player_hit_controller + 0x60);
    }

    auto get_aiming_data()
    {
        uintptr_t _aim_controller = * (uintptr_t *) ((uintptr_t) this + 0x60);
        return *reinterpret_cast<aiming_data **>((uintptr_t) _aim_controller + 0x98);
    }

    static void (*orig_update)(axlebolt::player_controller *);
};

void (*player_controller::orig_update)(axlebolt::player_controller *){};

struct player_manager
{
    il2cpp::klass *klass;
    void          *monitor;

    void *cached_ptr;
    void *m_CancellationTokenSource;

    bool initialized;
    int  _playerViewId;

    il2cpp::dictionary<int, player_controller *> *_playerById;

    void *unknown;
    void *hashset_controllers;

    il2cpp::dictionary<int, uintptr_t> *materials;

    void *hashset_materials;
    void *instantiate_event;
    void *destroy_event;
    void *on_set_snapshot_event;

    il2cpp::array<uintptr_t> custom_controllers;

    bool is_instantiate_sent;

    player_controller *prefab;
    player_controller *controller;

    uintptr_t ct_pool;
    uintptr_t t_pool;
};

#include "controls.h"
#include "snapshot.h"