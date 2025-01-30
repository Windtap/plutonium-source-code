#include <cstdint>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <dlfcn.h>
#include <string>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <sys/mman.h>

#include "includes/plutonium.h"
#include "includes/log.h"
#include "includes/game.h"
#include "includes/utils.h"
#include "xdl/include/xdl.h"
#include "imgui.h"
#include "imgui_impl_android.h"
#include "imgui_impl_opengl3.h"
#include <typeinfo>
#include "includes/addr.h"
#include "plutonium/data/offsets.h"
#include "imgui_internal.h"

template<typename T>
struct array_t
{
    T     *_array;
    size_t _count;

    void add_size(size_t new_count)
    {
        T *new_array = (T *) mmap(nullptr, sizeof(T) * new_count + 4096, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        for (int i{}; i < new_count && i < this->_count; i++)
        {
            new_array[i] = this->_array[i];
        }

        if (_array)
        {
            munmap(this->_array, _count * sizeof(T) + 4096);
        }

        this->_array = new_array;
        this->_count = new_count;
    }

    void add(const T &item)
    {
        int new_count = this->_count + 1;
        add_size(new_count);

        this->_array[this->_count - 1] = item;
    }

    bool contains(T &item)
    {
        for (int i{}; i < this->_count; i++)
        {
            if (item == _array[i])
            {
                return true;
            }
        }

        return false;
    }

    array_t<T> *init(size_t size)
    {

    }
};

struct class_t
{
    const char *name;
};

class instance_t
{
private:
    class_t *_class;
    void    *_object;

public:
    void initialize(void *object, class_t *info)
    {
        _class  = info;
        _object = object;
    }

    const char *name()
    {
        return _class->name;
    }
};

struct map_t
{
    uintptr_t start;
    size_t size;

    bool is_used;
};

class allocator_t
{
private:
    std::vector<map_t> *maps;
    size_t allocated_memory_sz;

public:

    allocator_t *init()
    {
        maps = new std::vector<map_t>();

        size_t allocate_size = PAGE_SIZE * 100; // сорян это фулл кал надо переделывать
        void *mapped_addr = mmap(nullptr, allocate_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        madvise(mapped_addr, allocate_size, MADV_WILLNEED);

        if (mapped_addr != MAP_FAILED)
        {
            LOGD("mapped at: %p", mapped_addr);
            map_t map = {
                    .start   = (uintptr_t) mapped_addr,
                    .size    = 0,
                    .is_used = false
            };

            allocated_memory_sz = allocate_size;

            maps->push_back(map);
        }
        else
        {
            LOGE("mmap failed");
            abort();
        }

        return this;
    }

    void *allocate_memory(size_t size)
    {
        for (int i{}; i < maps->size(); i++)
        {
            auto &map = maps->at(i);
            size_t all_maps_size = map.start - maps->at(0).start + size;

            if (all_maps_size + map.size > allocated_memory_sz)
            {
                break;
            }

            if (map.is_used == false)
            {
                map.is_used = true;
                void *allocated_addr = (void *) (map.start + map.size);
                LOGD("allocated addr: %p, map start: %p", allocated_addr, map.start);

                map_t new_map = {
                        .start   = (uintptr_t) allocated_addr,
                        .size    = size,
                        .is_used = false
                };

                madvise((void *) new_map.start, new_map.size, MADV_WILLNEED);

                maps->push_back(new_map);
                return allocated_addr;
            }
        }

        LOGE("allocated failed. free memory not found");
        return nullptr;
    }

    template<typename T>
    T *aalloc(size_t elements)
    {
        return (T *) allocate_memory(elements * sizeof(T));
    }

    template<typename T>
    T *calloc()
    {
       return aalloc<T>(1);
    }
};

class il2cpp_t
{
    uintptr_t _address;
public:

    uintptr_t address()
    {
        if (!this->_address)
        {
            Library *libil2cpp = new Library("libil2cpp.so");
            this->_address     = libil2cpp->GetAddress();
            delete libil2cpp;
        }

        return this->_address;
    }

    bool is_loaded()
    {
        Library *libunity = new Library("libil2cpp.so");
        bool is_loaded = libunity->Loaded();

        delete libunity;
        return is_loaded;
    }
};

class unity_t
{
    uintptr_t _address;
public:

    uintptr_t address()
    {
        if (!this->_address)
        {
            Library *libunity = new Library("libunity.so");
            this->_address    = libunity->GetAddress();
            delete libunity;
        }

        return this->_address;
    }

    bool is_loaded()
    {
        Library *libunity = new Library("libunity.so");
        bool is_loaded = libunity->Loaded();

        delete libunity;
        return is_loaded;
    }
};

std::atomic<struct plutonium_t *> _g_cheat{};
#include "il2cpp/il2cpp_api.h"

namespace engine
{
    struct players;
    struct aimbot;
}

struct plutonium_t
{
    allocator_t *_allocator;
    il2cpp_t   *_il2cpp;
    unity_t    *_unity;

    struct itcu_t
    {
    private:
        plutonium_t *_cheat;

    public:
        il2cpp::klass *find_class(const char *image_name, const char *class_name)
        {
            il2cpp::domain *il2cpp_domain = il2cpp::domain::get();

            if (il2cpp_domain)
            {
                void *iter{};
                while (il2cpp::assembly *assembly = il2cpp_domain->get_assemblies(&iter))
                {
                    il2cpp::image *image = assembly->get_image();

                    if (image_name == nullptr || strstr(image->name, image_name))
                    {
                        for (int i{}; i < image->typeCount; i++)
                        {
                            auto klass = image->get_class(i);
                            if (klass)
                            {
                                if (!strcmp(klass->_1.name, class_name))
                                {
                                    return klass;
                                }
                            }
                        }
                    }
                }
            }

            return nullptr;
        }

        il2cpp::method_info *find_method(il2cpp::klass *klass, const char *method_name, int params = -1)
        {
            void *iter{nullptr};
            * (void **) &iter = nullptr;

            while (il2cpp::method_info *method = klass->il2cpp_class_get_methods(&iter))
            {
                if (!strcmp(method->name, method_name) && (params == method->parameters_count || params == -1))
                {
                    return method;
                }
            }

            LOGE("method %s not found in %s", method_name, klass->_1.name);

            return nullptr;
        }

        template<typename H, typename O>
        bool method_hook(il2cpp::klass *klass, const char *method_name, H hook, O orig = nullptr)
        {
            if (klass && method_name && hook)
            {
                il2cpp::method_info* method = find_method(klass, method_name);

                if (method)
                {
                    void *&method_ptr = method->methodPointer;

                    if (method_ptr)
                    {
                        if (orig)
                        {
                            * (void **) orig = method_ptr;
                        }

                        method_ptr = (void *) hook;

                        return true;
                    }
                }
            }

            return false;
        }

        template<typename H, typename O>
        bool vmethod_hook(il2cpp::klass *klass, const char *method_name, H hook, O orig = nullptr, int method_count = -1)
        {
            if (!method_name|| !klass) { return false; }

            Il2CppClass_1     &_1                  = klass->_1;
            Il2CppClass_2     &_2                  = klass->_2;
            VirtualInvokeData *virtual_table_array = klass->vtable;

            for (uint16_t i{}; i < 9999; i++)
            {
                auto& virtual_table = virtual_table_array[i];
                void* methodPointer = (void*)virtual_table.methodPtr;
                const MethodInfo* &method = virtual_table.method;

                if (!method || !virtual_table.methodPtr)
                {
                    sleep(2);
                }

                const char* name = method->name;

                if (!strcmp(name, method_name))
                {
                    uint8_t count_of_parameters = method->parameters_count;

                    if (count_of_parameters > method_count && method_count >= 0)
                    {
                        continue;
                    }

                    if (orig)
                    {
                        * (void **) orig = methodPointer;
                    }

                    virtual_table.methodPtr = (Il2CppMethodPointer)hook;

                    return true;
                }
            }

            return false;
        }

        itcu_t *init(plutonium_t *cheat)
        {
            _cheat = cheat;

            return this;
        }
    };

    itcu_t *_itcu;
    engine::players *_players;
    engine::aimbot *_aimbot;

    void initialize()
    {
        _allocator = new allocator_t();
        this->_allocator->init();

        _il2cpp = this->_allocator->calloc<il2cpp_t>();
        _unity  = this->_allocator->calloc<unity_t>();

        while (!_il2cpp->is_loaded() || !_unity->is_loaded())
        {
            sleep(1);
        }

        _itcu   = this->_allocator->calloc<itcu_t>()->init(this);
    }
};


namespace menu
{
    plutonium_t *_cheat;
    #include "gui/menu/vars.h"

    cvars &cvars::vars()
    {
        static cvars _vars = cvars();

        return _vars;
    }

    static cvars &vars = cvars::vars();
}

namespace menu_includes
{
#include "plutonium/hooks/plthook.h"
}

#include "unity/UnityStructures.h"
#include "sdk/sdk.h"
#include "bypass.h"
#include "engine/world_to_screen.h"
#include "engine/chams.h"
#include "plutonium/engine/engine_players.h"
#include "plutonium/engine/skinchanger.h"
#include "gui/gui.h"
#include "plutonium/engine/bots/aimbot.h"
#include "plutonium/engine/exploits/exploits.h"
#include "plutonium/engine/engine.h"

const char *il2cpp::string::c_str()
{
    static plutonium_t *_cheat = _g_cheat.load();

    static il2cpp::klass       *marshal = _cheat->_itcu->find_class(nullptr, "Marshal");
    static il2cpp::method_info *method  = _cheat->_itcu->find_method(marshal, "StringToHGlobalAnsi", 1);

    typedef const char *(*to_cstr_t)(void *);
    static auto to_cstr = (to_cstr_t) method->methodPointer;

    return to_cstr(this);
}

void hack_prepare()
{
    //предупреждаю что код говно ну чисто что-то спастить я думаю сойдет
    plutonium_t *_cheat = new plutonium_t();
    sleep(10);

    _cheat->initialize();
    il2cpp_t *&_il2cpp = _cheat->_il2cpp;
    unity_t  *&_unity  = _cheat->_unity;

    LOGD("il2cpp_t: %p", _il2cpp);
    uintptr_t il2cpp_address = _il2cpp->address();
    uintptr_t unity_address  = _unity->address();
    LOGD("il2cpp address: %p, unity address: %p", il2cpp_address, unity_address);

    _g_cheat.store(_cheat);

    menu::vars.bools.enable_bone = _cheat->_allocator->template aalloc<bool>(6);

    engine::players  *_players = engine::players::create();
    LOGD("players: %p", _players);
    _cheat->_players = _players;

    engine::aimbot  *_aimbot = engine::aimbot::create();
    _cheat->_aimbot = _aimbot;

    il2cpp :: init(_cheat->_il2cpp->address());
    bypass :: init(*_cheat);
    menu   :: init(*_cheat);
    engine :: init(*_cheat);
}
