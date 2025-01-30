struct photon_view
{
    il2cpp::klass *klass;
    void          *monitor;


};

struct hashtable : il2cpp::dictionary<il2cpp::string *, uintptr_t>
{
    template<typename T>
    T get_value(const char *key)
    {
        for (int i{}; i < this->count; i++)
        {
            entry &_entry = this->entries->items[i];
            auto   _key = _entry.key;

            if (!_key) { continue; }
            const char *c_key = _key->c_str();
            if (!c_key) { continue; }

            if (!strcmp(c_key, key))
            {
                auto value = _entry.value;

                if (value)
                {
                    return * (T *) (value + 16);
                }
            }
        }

        return {};
    }
};

struct photon_player
{
    il2cpp::klass *klass;
    void          *monitor;

    uintptr_t iLogger;
    int       actor;

    il2cpp::string* name;
    il2cpp::string* user_id;

    bool is_local;
    bool is_inactive;

    il2cpp::dictionary<Il2CppObject *, Il2CppObject *> *_properties;

    uintptr_t tag;

    const char *get_nick()
    {
        auto name = * (il2cpp::string **) ((uintptr_t) this + 0x20);

        return name->c_str();
    }

    bool is_untouchable()
    {
        auto properties = * (hashtable **) ((uintptr_t) this + 0x38);
        if (properties)
        {
            return properties->get_value<bool>("untouchable");
        }

        return 0;
    }

    int get_health()
    {
        auto properties = * (hashtable **) ((uintptr_t) this + 0x38);
        if (properties)
        {
            return properties->get_value<int>("health");
        }

        return 0;
    }
};