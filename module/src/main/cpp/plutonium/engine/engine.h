namespace engine
{
    constexpr const char *on_occlusion_became_visible   = "HFCEDAECDAAEGDB";
    constexpr const char *on_occlusion_became_invisible = "EFGEFGHEBGDFCCA";
    constexpr const char *execute_commnads              = "BFHDEEHAGCEHEBB";
    constexpr const char *player_serialize              = "AFEGEDFCABEEHHC";


    void init(plutonium_t &_cheat)
    {
        il2cpp_addr = _cheat._il2cpp->address();
        _itcu       = _cheat._itcu;

        axlebolt :: init(_cheat);
        unity    :: init(_cheat);

        il2cpp::klass *fmod_manager_class = _itcu->find_class(nullptr, "FMODRuntimeManagerOnGUIHelper");
        _itcu->method_hook(fmod_manager_class, "OnGUI", fmod_runtime_manager_ongui_helper__ongui, &orig_fmod_runtime_manager_ongui_helper__ongui);

        auto attached_thread = il2cpp::thread::attach();
        il2cpp::klass *game_object_class  = _itcu->find_class(nullptr, "GameObject");

        auto game_obj = il2cpp::object::create<unity::game_object>(game_object_class);
        game_obj->create(il2cpp::string::create("plutonium"));

        auto component_type = unity::type::from_class(fmod_manager_class);
        game_obj->add_component(component_type);
        attached_thread->detach();

        il2cpp::klass *player_class = _itcu->find_class(nullptr, "PlayerController");
        _itcu->method_hook(player_class, "Update", player_controller__update, &axlebolt::player_controller::orig_update);
        _itcu->vmethod_hook<void *, void *>(player_class, on_occlusion_became_invisible, _itcu->find_method(player_class, on_occlusion_became_visible)->methodPointer);

        il2cpp::klass *gun_class = _itcu->find_class(nullptr, "GunController");
        _itcu->vmethod_hook(gun_class, execute_commnads, gun_controller__execute_commands, &axlebolt::gun_controller::execute_commands);

        il2cpp::klass *snapshot_class = _itcu->find_class(nullptr, "EBDGEEAAEDFECGE");
        _itcu->vmethod_hook(snapshot_class, player_serialize, hk_player_snapshot__serialize, &axlebolt::player_snapshot::serialize);

        void *call_raycast = (void *) (_g_cheat.load()->_il2cpp->address() + 0x538AB20);
        while (!*(void**)call_raycast)
        {
            sleep(1);
        }

        bypass::icall_hook(call_raycast, "UnityEngine.PhysicsScene::Internal_Raycast_Injected(UnityEngine."
                                         "PhysicsScene&,UnityEngine.Ray&,System.Single,UnityEngine.Raycast"
                                         "Hit&,System.Int32,UnityEngine.QueryTriggerInteraction)",
                           hk_raycast, &orig_raycast);
    }
}