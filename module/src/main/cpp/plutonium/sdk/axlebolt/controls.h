struct touch_control
{
    il2cpp::klass *klass;
    void          *monitor;
};

struct player_controls
{
    il2cpp::klass *klass;
    void          *monitor;

    void *cache_ptr;
    void *cancel_token;
    uintptr_t source_type;
    uintptr_t mouse_control;

    player_controller *player;
    static void (*orig_update)(axlebolt::player_controls *);
};

void (*player_controls::orig_update)(axlebolt::player_controls *){};
