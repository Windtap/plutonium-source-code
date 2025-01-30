namespace logger
{
    inline void in_sdk(const char *function_name, const char *class_name)
    {
        LOGD("[plutonium_api info] called %s in class %s", function_name, class_name);
    }

    inline void in_game(const char *function_name, const char *class_name)
    {
        LOGD("[plutonium_api info] game called %s in class %s", function_name, class_name);
    }
}


namespace unity
{
    plutonium_t *_cheat;
    #include "unity/basic.h"

    void init(plutonium_t &cheat)
    {
        _cheat = &cheat;
    }
}

namespace axlebolt
{
    plutonium_t *_cheat;

    #include "axlebolt/safe_values.h"
    #include "axlebolt/photon.h"
    #include "axlebolt/occlusion.h"
    #include "axlebolt/player.h"
    #include "axlebolt/weapon.h"
    #include "axlebolt/bolt.h"

    void init(plutonium_t &cheat)
    {
        _cheat = &cheat;
    }
}
