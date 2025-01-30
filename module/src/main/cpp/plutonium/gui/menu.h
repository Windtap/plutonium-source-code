class GUI
{
private:
    unity::screen screen;
public:

    inline void draw_fov()
    {
        if (vars.bools.enable_aimbot)
        {
            if (vars.integers.fov_type == 0)
            {
                ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(screen.get_width() * 0.5f, screen.get_height() * 0.5f), vars.floats.aimbot_fov, vars.colors.fov_color, 85, 3.f);
            }
            else if (vars.integers.fov_type == 1)
            {
                float x = screen.get_width() * 0.5f - vars.floats.aimbot_fov * 0.5f;
                float y = screen.get_height() * 0.5f - vars.floats.aimbot_fovy * 0.5f;

                float w = x + vars.floats.aimbot_fov;
                float h = y + vars.floats.aimbot_fovy;

                ImGui::GetBackgroundDrawList()->AddRect(ImVec2(x, y), ImVec2(w, h), vars.colors.fov_color, 0, 0, 3.f);
            }
        }
    }

    EGLBoolean present_frame(EGLDisplay dpy, EGLSurface surface)
    {
        ImGuiIO &io = ImGui::GetIO();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        static cvars *vars = &menu::vars;
        static auto &ui = c_ui;

        ui->render((void *) vars);
        draw_esp(menu::_cheat, screen);
        draw_fov();


        ImGui::EndFrame();
        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        return 1;
    }

    GUI *init()
    {
        LOGI("setuping");

        screen = unity::screen::get_screen();

        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();

        io.DisplaySize = ImVec2((float)screen.get_width(), (float)screen.get_height());

        // Setup Dear ImGui style
        // Setup Platform/Renderer backends
        ImGui_ImplOpenGL3_Init("#version 100");

        c_ui->initialize();
        c_ui->initialize_fonts();

        // We load the default font with increased size to improve readability on many devices with "high" DPI.
        ImFontConfig font_cfg;
        ImFontConfig font_cfg2;
        font_cfg.SizePixels = 22.f;
        fonts::SFPro_font = io.Fonts->AddFontFromMemoryCompressedBase85TTF((const char *) fonts::compressed_data_base85, 22.f, &font_cfg);

        font_cfg2.SizePixels = 22.f;
        font_cfg2.GlyphRanges = io.Fonts->GetGlyphRangesCyrillic();

        fonts::pixel_font = io.Fonts->AddFontFromMemoryTTF(fonts::font_pixel, sizeof(fonts::font_pixel), 22.f, &font_cfg2);
        fonts::tahoma = io.Fonts->AddFontFromMemoryTTF(fonts::tahoma_font, sizeof(fonts::tahoma_font), 22.f, &font_cfg2);

        return this;
    }
};