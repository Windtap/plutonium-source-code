#include "il2cpp_structs.h"

namespace il2cpp
{
    static uintptr_t il2cpp_addr;
    struct assembly;
    struct klass;

    struct domain
    {
        static domain *get()
        {
            return ((domain *(*)()) (il2cpp_addr + offsets::il2cpp_domain_get_offsets))();
        }

        assembly *get_assemblies(void **iter)
        {
            return ((assembly *(*)(void *, void *)) (il2cpp_addr + offsets::mono_domain_get_assemblies_offsets))(this, iter);
        }
    };

    struct thread
    {
        static thread *attach()
        {
            return ((thread *(*)(void *)) (il2cpp_addr + offsets::il2cpp_thread_attach))(domain::get());
        }

        static void attach(void (*function)())
        {
            auto thread = attach();

            function();

            thread->detach();
        }

        void detach()
        {
            ((void (*)(void *)) (il2cpp_addr + offsets::il2cpp_thread_detach))(this);
        }
    };

    struct image
    {
        const char* name;
        const char *nameNoExt;
        void* assembly;
        uint32_t typeCount;
        uint32_t exportedTypeCount;
        uint32_t customAttributeCount;
        void* metadataHandle;
        void * nameToClassHashTable;
        const void* codeGenModule;
        uint32_t token;
        uint8_t dynamic;
        int ref_count;
        void *raw_data_handle;
        void *raw_data;
        void *unk;
        void *unk_2;

        klass *get_class(int type)
        {
            void *v1 = ((void *(*)(void *, int)) (il2cpp_addr + offsets::il2cpp_image_get_class_obj))(this, type);
            return ((klass *(*)(void *)) (il2cpp_addr + offsets::il2cpp_image_get_class))(v1);
        }
    };

    struct method_info
    {
        void* methodPointer;
        Il2CppMethodPointer virtualMethodPointer;
        InvokerMethod invoker_method;
        const char* name;
        Il2CppClass *klass;
        const Il2CppType *return_type;
        const Il2CppType** parameters;
        union
        {
            const Il2CppRGCTXData* rgctx_data;
            const void* methodMetadataHandle;
        };
        union
        {
            const void* genericMethod;
            const void* genericContainerHandle;
        };
        uint32_t token;
        uint16_t flags;
        uint16_t iflags;
        uint16_t slot;
        uint8_t parameters_count;
        uint8_t bitflags;
    };

    struct klass
    {
        Il2CppClass_1 _1;
        void* static_fields;
        Il2CppRGCTXData* rgctx_data;
        Il2CppClass_2 _2;
        VirtualInvokeData vtable[255];

        method_info *il2cpp_class_get_methods(void **iter)
        {
            return ((method_info *(*)(void *, void **)) (il2cpp_addr + offsets::il2cpp_class_get_methods_offsets))(this, iter);
        }

        bool is_value_type()
        {
            return *(int *)((uintptr_t) this + 40) >> 31;
        }
    };

    struct assembly
    {
        image *get_image()
        {
            return * (image **) this;
        }
    };

    struct object
    {
        template<typename T>
        static T *create(il2cpp::klass *klass)
        {
            return ((T* (*)(const void *)) (il2cpp_addr + offsets::il2cpp_object_new))(klass);
        }
    };

    template<typename T>
    struct array
    {
        il2cpp::klass *klass;
        void          *monitor;

        Il2CppArrayBounds *bounds;
        uint32_t           count;
        T                  items [0];

        template<typename V>
        static array<V> *create(il2cpp::klass *klass, int count)
        {
            return ((array<V> * (*)(const void *, int)) (il2cpp_addr + /*update this*/0x0))(klass, count);
        }
    };

    template<typename T>
    struct list
    {
        il2cpp::klass *klass;
        void          *monitor;

        array<T>          *array;
        uint32_t           count;
    };

    template<typename TKey, typename TValue>
    struct dictionary
    {
        struct entry
        {
            [[maybe_unused]] int hash_code, next;
            TKey key;
            TValue value;
        };

        il2cpp::klass *klass;
        void          *monitor;

        [[maybe_unused]] array<int> *buckets;
        array<entry> *entries;
        int count;
        int version;
        [[maybe_unused]] int freeList;
        [[maybe_unused]] int freeCount;
        void *compare;
        array<TKey> *keys;
        array<TValue> *values;
        [[maybe_unused]] void *syncRoot;

        TValue get_value(TKey key)
        {
            for (int i{}; i < entries->count; i++)
            {
                if (entries->items[i].key == key)
                {
                    return entries->items[i].value;
                }
            }

            //LOGD("not found value: %d", key);

            return NULL;
        }

        void set_value(TKey key, TValue value)
        {
            for (int i{}; i < entries->count; i++)
            {
                if (entries->items[i].key == key)
                {
                    entries->items[i].value = value;
                }
            }
        }
    };

    struct string
    {
        il2cpp::klass *klass;
        void *monitor;
        int length;

        static string *create(const char *str)
        {
            return ((string * (*)(const void *)) (il2cpp_addr + 0x2136D1C))(str);
        }

        const char *c_str();
    };

    struct type
    {
        object *get_object()
        {
            return ((object * (*)(void *type_klass)) (il2cpp_addr + 0x20E60A0))(this);
        }
    };

    struct icall
    {
        const char *dll_name;
        size_t dll_name_len;
        const char *function_name;
        size_t function_name_len;
        size_t flags = 0x200000000;

        [[maybe_unused]]
        icall(const char *dll, const char *function)
        {
            dll_name = dll;
            function_name = function;

            dll_name_len = strlen(dll);
            function_name_len = strlen(function);
        }

        void *resolve_icall()
        {
            return ((void *(*)(void *)) (il2cpp_addr + 0x20EFA98))(this);
        }

        void *resolve_icall_unity()
        {
            return ((void *(*)(const char *)) (il2cpp_addr + 0x20EF46C))(this->function_name);
        }

    };

    void init(uintptr_t _il2cpp_addr)
    {
        il2cpp_addr = _il2cpp_addr;
    }
};