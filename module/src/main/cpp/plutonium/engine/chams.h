namespace engine
{
    namespace chams
    {
#include "chams/shader.h"

        void set_player(axlebolt::player_controller *player)
        {
            static unity::shader *wireframe_shader = unity::shader::find(il2cpp::string::create(shader));
            if (!player || !wireframe_shader->is_valid()) { return; }

            unity::skinned_mesh_renderer *renderer = player->get_renderer();
            if (!renderer) { return; }

            unity::material *material = renderer->get_material();
            material->set_shader(wireframe_shader);
        }
    }
}