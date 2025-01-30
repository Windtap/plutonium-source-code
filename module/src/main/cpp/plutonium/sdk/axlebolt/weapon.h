struct weapon_command
{
    alignas(1) bool to_fire;
    alignas(1) bool to_aim;
    alignas(1) bool to_reload;
    alignas(1) bool to_action;
    alignas(2) bool to_inspection;
};

struct recoil_control
{
    il2cpp::klass *klass;
    void          *monitor;

    float shoot_time;
    float progress;
    float prev_shoot_progress;
    Vector2 previous_point;
    Vector2 current_point;

    float approach_delta_dist;
    Vector2 relative_dispersion;
    float local_time;
    uintptr_t recoil_data;
    uintptr_t gun_parameters;
    uintptr_t recoil_parameters;

    safe_float radius;
    safe_float min_degree;
    safe_float max_degree;

    float deceleration_coeff;
};

struct gun_controller
{
    il2cpp::klass *klass;
    void          *monitor;

    void *cached_ptr;
    void *m_CancellationTokenSource;

    player_controller *player;
    uintptr_t mecanim;
    uintptr_t weapon_animator;
    uintptr_t weapon_animator_parameters;
    int       state;

    inline recoil_control *get_recoil()
    {
        return * (recoil_control **) ((uint64_t) this + 0x140);
    }

    void set_shoot_point(Vector3 point)
    {
        recoil_control *recoil = get_recoil();

        if (recoil)
        {
            recoil->approach_delta_dist = 0.f;
            recoil->deceleration_coeff = 0.f;
            //recoil->max_degree.set_value(0.f);
            //recoil->min_degree.set_value(0.f);
            //recoil->radius.set_value(0.f);

            recoil->prev_shoot_progress = 0;
            recoil->relative_dispersion = Vector2::zero;

            recoil->current_point = Vector2(0.f, 0.f);
            recoil->previous_point = Vector2(0.f, 0.f);

            player->get_aiming_data()->current_aim_angle.x = point.x;
            player->get_aiming_data()->current_euler_angle.x = point.y;

            unity::transform *main_camera = * ( unity::transform **) ((uintptr_t) this + 0x188);

            main_camera->set_euler(point);
        }
    }

    static void (*execute_commands)(gun_controller *, weapon_command, float, float);
};

void (*gun_controller::execute_commands)(gun_controller *, weapon_command, float, float);