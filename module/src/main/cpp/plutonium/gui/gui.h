#include "touches.h"
#include "shadow.h"

namespace fonts
{
#include "plutonium/data/SFPro.h"
#include "plutonium/data/FontESP.h"

#include "plutonium/data/tahoma.h"

    ImFont *SFPro_font;
    ImFont *pixel_font;
    ImFont *tahoma;
}

#include "esp.h"
#include "menu/ui.hpp"
#include "skinchanger.h"

namespace menu
{
    #include "menu.h"

    int (*orig_GfxDeviceGLES_PresentFrame)(uintptr_t);
    int hk_GfxDeviceGLES_PresentFrame(uintptr_t instance)
    {
        static GUI      *gui     = _g_cheat.load()->_allocator->calloc<GUI>()->init();
        static uintptr_t context = _g_cheat.load()->_unity->address() + 0x1266888;

        EGLDisplay dpy     = * (EGLDisplay *) context;
        EGLSurface surface = * ((EGLSurface *) context + 2);

        gui->present_frame(dpy, surface);
        return orig_GfxDeviceGLES_PresentFrame(instance);
    }

    void init(plutonium_t &_cheat)
    {
        menu::_cheat = _g_cheat.load();

        LOGD("unity address: %p", _cheat._unity->address());

        void *present_frame_ptr = (void *) (_cheat._unity->address() + 0x11A1670);
        LOGD("Present Frame ptr: %p", present_frame_ptr);
        LOGD("Present Frame: %p", * (void **) present_frame_ptr);

        bypass::swap_ptr(present_frame_ptr, hk_GfxDeviceGLES_PresentFrame, &orig_GfxDeviceGLES_PresentFrame);
        sleep(1);
        void *delegate_input_touch = (void *) (_g_cheat.load()->_il2cpp->address() + offsets::DelegateTouchCountHook);
        while (!*(void**)delegate_input_touch)
        {
            sleep(1);
        }

        bypass::icall_hook(delegate_input_touch, "UnityEngine.Input::get_touchCount()", _Input$$get_touchCount, &Input$$get_touchCount);
    }
}