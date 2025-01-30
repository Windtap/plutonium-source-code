struct inventory
{
    struct item
    {
        struct properties;
        struct definition;

        il2cpp::klass *klass;
        void *monitor;

        int definition_id;
        int quantity;
        int flags;
        void *unk;
        properties *props;
        int id;

        struct definition
        {
            il2cpp::klass *klass;
            void *monitor;

            int id;
            il2cpp::string name;
        };

        struct properties
        {
            struct property
            {
                il2cpp::klass *klass;
                void *monitor;
            };

            il2cpp::klass *klass;
            void *monitor;

            il2cpp::dictionary<il2cpp::string *, property *> *properties;
        };
    };

    struct service
    {
        static service *get_instance()
        {
            static const char *inventory_service_name = "FAHBEFGGDBHBEEB";
            static il2cpp::klass *klass = _g_cheat.load()->_itcu->find_class(nullptr, inventory_service_name);
            static il2cpp::klass *parent = (il2cpp::klass *) klass->_1.parent;
            static void *static_fields = parent->static_fields;

            return * (service **) static_fields;
        }

        auto get_definitions()
        {
            return * (il2cpp::dictionary<int, item::definition *> **) ((uintptr_t) this + 0xE8);
        }

        auto get_items()
        {
            return * (il2cpp::dictionary<int, item *> **) ((uintptr_t) this + 0xF8);
        }

        item::definition *get_definition(int id)
        {
            auto definitions = get_definitions();
            if (definitions && !definitions->monitor)
            {
                return definitions->get_value(id);
            }

            return nullptr;
        }
    };
};