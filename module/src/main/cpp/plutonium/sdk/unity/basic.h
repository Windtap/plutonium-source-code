template<typename T>
struct nullable
{
    T value;
    bool has_value;
};

struct object
{
};

struct component;
struct type
{
    static type *from_class(il2cpp::klass *klass)
    {
        il2cpp::type *_type = (il2cpp::type *) ((uintptr_t) klass + 0x20);

        return (type *) _type->get_object();
    }
};

struct game_object
{
    il2cpp::klass *klass;

    game_object *create(il2cpp::string *name)
    {
        static il2cpp::klass *klass         = _cheat->_itcu->find_class("UnityEngine", "GameObject");
        static void*          function_addr = _cheat->_itcu->find_method(klass, "Internal_CreateGameObject")->methodPointer;

        return ((game_object* (*)(...)) function_addr)(this, name);
    }

    void add_component(type *type)
    {
        static il2cpp::klass *klass         = _cheat->_itcu->find_class("UnityEngine", "GameObject");
        static void*          function_addr = _cheat->_itcu->find_method(klass, "Internal_AddComponentWithType")->methodPointer;

        return ((void (*)(...)) function_addr)(this, type);
    }

    template<class T>
    T *get_component(unity::type *type)
    {
        static void *function_addr = _cheat->_itcu->find_method(klass, "GetComponent", 1)->methodPointer;

        return ((T* (*)(...)) function_addr)(this, type);
    }
};

struct transform
{
    inline auto get_component()
    {
        return (component *) this;
    }

    unity::structures::Vector3 get_position()
    {
        //logger::in_sdk("get_position", "transform");

        static il2cpp::klass *klass         = _cheat->_itcu->find_class("UnityEngine", "Transform");
        static void*          function_addr = _cheat->_itcu->find_method(klass, "get_position")->methodPointer;

        return ((unity::structures::Vector3 (*)(...)) function_addr)(this);
    }

    Quaternion get_rotation()
    {
        //logger::in_sdk("get_rotation", "transform");

        static il2cpp::klass *klass         = _cheat->_itcu->find_class("UnityEngine", "Transform");
        static void*          function_addr = _cheat->_itcu->find_method(klass, "get_rotation")->methodPointer;

        return ((Quaternion (*)(...)) function_addr)(this);
    }

    Vector3 get_euler()
    {
        //logger::in_sdk("get_rotation", "transform");

        static il2cpp::klass *klass         = _cheat->_itcu->find_class("UnityEngine", "Transform");
        static void*          function_addr = _cheat->_itcu->find_method(klass, "get_eulerAngles")->methodPointer;

        return ((Vector3 (*)(...)) function_addr)(this);
    }

    Vector3 transform_point(Vector3 point)
    {
        static il2cpp::klass *klass         = _cheat->_itcu->find_class("UnityEngine", "Transform");
        static void*          function_addr = _cheat->_itcu->find_method(klass, "TransformPoint")->methodPointer;

        return ((Vector3 (*)(void *, Vector3)) function_addr)(this, point);
    }

    void set_euler(Vector3 euler)
    {
        //logger::in_sdk("get_rotation", "transform");

        static il2cpp::klass *klass         = _cheat->_itcu->find_class("UnityEngine", "Transform");
        static void*          function_addr = _cheat->_itcu->find_method(klass, "set_eulerAngles")->methodPointer;

        return ((void (*)(void *, Vector3)) function_addr)(this, euler);
    }
};

struct component
{
    transform *get_transform()
    {
        //logger::in_sdk("get_transform", "component");

        static il2cpp::klass *klass         = _cheat->_itcu->find_class("UnityEngine", "Component");
        static void*          function_addr = _cheat->_itcu->find_method(klass, "get_transform")->methodPointer;

        return ((transform *(*)(...)) function_addr)(this);
    }

    game_object *get_game_object()
    {
        //logger::in_sdk("get_transform", "component");

        static il2cpp::klass *klass         = _cheat->_itcu->find_class("UnityEngine", "Component");
        static void*          function_addr = _cheat->_itcu->find_method(klass, "get_gameObject")->methodPointer;

        return ((game_object *(*)(...)) function_addr)(this);
    }

    template<class T>
    T *get_component(type *type)
    {
        static il2cpp::klass *klass         = _cheat->_itcu->find_class("UnityEngine", "Component");
        static void*          function_addr = _cheat->_itcu->find_method(klass, "GetComponent", 1)->methodPointer;

        return ((T *(*)(...)) function_addr)(this, type);
    }
};

struct camera
{
    component *get_component()
    {
        return (component *) this;
    }

    float get_fieldOfView()
    {
        static il2cpp::klass *klass         = _cheat->_itcu->find_class("UnityEngine", "Camera");
        static void*          function_addr = _cheat->_itcu->find_method(klass, "get_fieldOfView")->methodPointer;

        return ((float (*)(...)) function_addr)(this);
    }

    static camera *get_main()
    {
        static il2cpp::klass *klass         = _cheat->_itcu->find_class("UnityEngine", "Camera");
        static void*          function_addr = _cheat->_itcu->find_method(klass, "get_main")->methodPointer;

        camera *_camera = ((camera *(*)()) function_addr)();
        return _camera;
    }
};

struct time
{
    static float get_deltaTime()
    {
        static il2cpp::klass *klass         = _cheat->_itcu->find_class("UnityEngine", "Time");
        static void*          function_addr = _cheat->_itcu->find_method(klass, "get_deltaTime")->methodPointer;

        return ((float (*)(...)) function_addr)();
    }
};


struct screen
{
private:
    int height;
    int width;

public:

    static screen &get_screen()
    {
        static plutonium_t *_cheat = _g_cheat.load();
        static screen       screen{};

        il2cpp::klass *screen_class = _cheat->_itcu->find_class(nullptr, "Screen");
        typedef int (*get_screen_function_t)();

        LOGD("screen class: %p", screen_class);

        auto get_height = (get_screen_function_t) _cheat->_itcu->find_method(screen_class, "get_height")->methodPointer;
        auto get_width  = (get_screen_function_t) _cheat->_itcu->find_method(screen_class, "get_width")->methodPointer;

        LOGD("get_height: %p\nget_width: %p", get_height, get_width);

        screen.height = get_height();
        screen.width  = get_width();

        LOGI("height: %d, width: %d", screen.height, screen.width);

        return screen;
    }

    inline int get_width()
    {
        return width;
    }

    inline int get_height()
    {
        return height;
    }
};

struct physics
{
    static bool linecast(Vector3 start, Vector3 end, int mask)
    {
        static il2cpp::klass *klass         = _cheat->_itcu->find_class("UnityEngine", "Physics");
        static void*          function_addr = _cheat->_itcu->find_method(klass, "Linecast")->methodPointer;

        return ((bool (*)(Vector3 start, Vector3 end, int mask)) function_addr)(start, end, mask);
    }
};

struct bounds
{
    unity::structures::Vector3 center;
    unity::structures::Vector3 extents;

    unity::structures::Vector3 max()
    {
        return center + extents;
    }

    unity::structures::Vector3 min()
    {
        return center - extents;
    }

    unity::structures::Vector3 size()
    {
        return extents * 2.f;
    }

    void encapsulate(Vector3 point)
    {
        Vector3 new_min_point = Vector3(std::min(min().x, point.x), std::min(min().y, point.y), std::min(min().z, point.z));
        Vector3 new_max_point = Vector3(std::min(max().x, point.x), std::min(max().y, point.y), std::min(max().z, point.z));

        extents = (new_max_point - new_min_point) * 0.5f;
        center = new_min_point + extents;
    }

    void encapsulate(unity::bounds &bounds)
    {
        encapsulate(bounds.min());
        encapsulate(bounds.max());
    }
};

struct material
{
    il2cpp::klass *klass;

    void set_shader(void *shader)
    {
        static void* function_addr = _cheat->_itcu->find_method((il2cpp::klass *) klass->_1.parent, "get_bounds")->methodPointer;

        return ((void (*)(void*, void *)) function_addr)(this, shader);
    }
};

struct mesh;

struct skinned_mesh_renderer
{
    il2cpp::klass *klass;

    bounds get_bounds()
    {
        static void* function_addr = _cheat->_itcu->find_method((il2cpp::klass *) klass->_1.parent, "get_bounds")->methodPointer;

        return ((bounds (*)(void*)) function_addr)(this);
    };

    mesh* get_shared_mesh()
    {
        static void* function_addr = _cheat->_itcu->find_method((il2cpp::klass *) klass, "get_sharedMesh")->methodPointer;

        return ((mesh* (*)(void*)) function_addr)(this);
    };

    material *get_material()
    {
        static void* function_addr = _cheat->_itcu->find_method((il2cpp::klass *) klass->_1.parent, "GetMaterial")->methodPointer;

        return ((material *(*)(void*)) function_addr)(this);
    };

    void bake_mesh(mesh *mesh)
    {
        static void* function_addr = _cheat->_itcu->find_method((il2cpp::klass *) klass, "BakeMesh", 2)->methodPointer;

        return ((void (*)(void*, void *, bool)) function_addr)(this, mesh, true);
    };
};

struct collider
{
    il2cpp::klass *klass;

    bounds get_bounds()
    {
        static void* function_addr = _cheat->_itcu->find_method((il2cpp::klass *) klass->_1.parent, "get_bounds")->methodPointer;
        auto a = 1;

        return ((bounds (*)(void*)) function_addr)(this);
    }
};

struct character_controller
{
    il2cpp::klass *klass;

    bounds get_bounds()
    {
        static void* function_addr = _cheat->_itcu->find_method((il2cpp::klass *) klass->_1.parent, "get_bounds")->methodPointer;

        return ((bounds (*)(void*)) function_addr)(this);
    }
};

struct shader
{
    il2cpp::klass *klass;
    void *monitor;

    static shader *find(il2cpp::string *shader_name)
    {
        il2cpp::klass *klass         = _cheat->_itcu->find_class(nullptr, "Shader");
        void          *function_addr = _cheat->_itcu->find_method(klass, "Find")->methodPointer;

        auto _shader = ((shader *(*)(void *)) function_addr)(shader_name);
        LOGD("created shader: %p", _shader);

        return _shader;
    }

    inline bool is_valid()
    {
        return this && monitor && klass;
    }
};

struct mesh
{
    il2cpp::klass *klass;

    bounds get_bounds()
    {
        static void* function_addr = _cheat->_itcu->find_method((il2cpp::klass *) klass, "get_bounds")->methodPointer;

        return ((bounds (*)(void*)) function_addr)(this);
    };

    static mesh *create()
    {
        static il2cpp::klass *mesh_class = _cheat->_itcu->find_class(nullptr, "Mesh");
        static void* function_addr = _cheat->_itcu->find_method(mesh_class, ".ctor", 0)->methodPointer;

        mesh *mesh_instance = il2cpp::object::create<mesh>(mesh_class);
        ((void (*)(...)) function_addr)(mesh_instance);

        return mesh_instance;
    }
};