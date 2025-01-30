enum occlusion_state
{
    state_none = 0,
    invisible = 1,
    visible = 2
};

struct object_occludee
{
    il2cpp::klass *klass;
    void *monitor;

    unity::character_controller *get_character()
    {
        return * (unity::character_controller **) ((uintptr_t) this + 0x50);
    }

    void set_visible(bool is_visible)
    {
        //HDBAGEABGFACHBA
        static void* function_addr = _cheat->_itcu->find_method((il2cpp::klass *) klass->_1.parent, "HDBAGEABGFACHBA")->methodPointer;
        //LOGD("set visible : %p", function_addr);

        ((void (*)(void *, bool is_visible)) function_addr)(this, is_visible);
    }

    occlusion_state get_state()
    {
        return * (occlusion_state *) ((uintptr_t) this + 0x34);
    }

    void set_state(occlusion_state state)
    {
        * (occlusion_state *) ((uintptr_t) this + 0x34) = state;
    }

    bool visible()
    {
        return get_state() == occlusion_state::visible;
    }

    bool in_frustum_area()
    {
        return * (bool *) ((uintptr_t) this + 0x31);
    }
};

struct occlusion_control
{
    il2cpp::klass *klass;
    void *monitor;

    uintptr_t    m_cached;
    void        *ct_source;
    photon_view *pv_cache;

    il2cpp::dictionary<object_occludee *, int> *objects_by_id;
    il2cpp::list<object_occludee *>            *objects;


    static void (*update)(occlusion_control *);

    void on_occlusion_became_visible(object_occludee *occludee)
    {
        static void* function_addr = _cheat->_itcu->find_method(klass, "EDCEABFABHDBEDF")->methodPointer;
        //LOGD("get visible : %p", function_addr);

        ((void (*)(void *, void *)) function_addr)(this, occludee);
    }

    void set_ignore_raycasting(void *func)
    {
        static void* function_addr = _cheat->_itcu->find_method(klass, "BCAEACACCCHEFBA")->methodPointer;
        //LOGD("get visible : %p", function_addr);

        ((void (*)(void *, void *)) function_addr)(this, func);
    }
};

void (*occlusion_control::update)(occlusion_control *){};