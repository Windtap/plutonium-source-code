namespace bypass
{
    struct hook_info
    {
        void *ptr_addr;

        void *hook_addr;
        void *orig_addr;

        bool is_swap_hook;
    };

    std::vector<hook_info *> hooked_funcs;

    template<class h, class o>
    void icall_hook(void *delegate_addr, const char *method_name, h hook, o orig, const char *dll = "libunity")
    {
        hook_info *info = _g_cheat.load()->_allocator->calloc<hook_info>();

        info->ptr_addr  = delegate_addr;
        info->hook_addr = (void *) hook;
        info->is_swap_hook = false;

        auto *_icall    = new il2cpp::icall(dll, method_name);
        void *orig_addr;

        if (!strcmp(dll, "libunity"))
        {
            orig_addr = _icall->resolve_icall_unity();
        }
        else
        {
            orig_addr = _icall->resolve_icall();
        }

        if (orig) { * (void **) orig = orig_addr; }

        info->orig_addr = (void *) orig_addr;

        hooked_funcs.push_back(info);

        * (void **) delegate_addr = (void *) hook;
    }

    template<class h, class o>
    void swap_ptr(void *addr, h hk, o orig)
    {
        hook_info *info = _g_cheat.load()->_allocator->calloc<hook_info>();
        info->ptr_addr  = addr;
        info->hook_addr = (void *) hk;
        info->orig_addr = * (void **) addr;

        info->is_swap_hook = true;
        hooked_funcs.push_back(info);

        menu_includes::hook(addr, (void *) hk, (void **) orig);
    }

    void *(*orig_shared_getrr)(const char *id, const char *report_arg);
    void *hk_shared_getrr(const char *id, const char *report_arg)
    {
        LOGD("reparg: %s", report_arg);

        for (int i{}; i < hooked_funcs.size(); i++)
        {
            auto info = hooked_funcs[i];

            if (info->is_swap_hook)
            {
                menu_includes::hook(info->ptr_addr, info->orig_addr, nullptr);
            }
            else
            {
                * (void **) info->ptr_addr = info->orig_addr;
            }

            LOGD("replaced to orig is swap hook: %d\nptr address: %p\norig address\nhook address", info->is_swap_hook, info->ptr_addr, info->orig_addr, info->hook_addr);
        }

        void *result = orig_shared_getrr(id, report_arg);

        for (int i{}; i < hooked_funcs.size(); i++)
        {
            auto info = hooked_funcs[i];

            if (info->is_swap_hook)
            {
                menu_includes::hook(info->ptr_addr, info->hook_addr, nullptr);
            }
            else
            {
                * (void **) info->ptr_addr = info->hook_addr;
            }

            LOGD("replaced to hook is swap hook: %d\nptr address: %p\norig address\nhook address", info->is_swap_hook, info->ptr_addr, info->orig_addr, info->hook_addr);
        }

        return result;
    }

    void init(plutonium_t &cheat)
    {
        void *getrr_delegate = (void *) (cheat._il2cpp->address() + 0x51186B8);
        icall_hook(getrr_delegate, "_Unwind_GetRR", hk_shared_getrr, &orig_shared_getrr, "libshared");
    }
}