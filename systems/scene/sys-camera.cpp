#include "sys-camera.h"
#include "flecs-world.h"
#include "cmp-window.h"

namespace Axiom
{
    namespace Scene
    {
        namespace Camera{
            void initialize(){
                auto Cam = g_world.entity("Camera");
                Cam.set(Cmp_Camera());
                Cam.set(Cmp_Transform());
                Cam.set(Cmp_Static());

                g_world.system<Cmp_Camera>("Update Camera")
                .kind(flecs::OnUpdate)
                .each([](flecs::entity e, Cmp_Camera& c){
                    auto* t = e.get_mut<Cmp_Transform>();
                    update_camera(e, *t, c);
                });
            }
            void update_camera(flecs::entity e, Cmp_Transform &t, Cmp_Camera &c){
                // Update Transform of Camera based on Mouse input
                // If mouse-left is being held then...
                // the camera rotates according to the x/y
                // If mouse is scrolled it translates according to z

                auto* mouse = g_world.get_mut<Cmp_Mouse>();
                if((mouse->buttons[GLFW_MOUSE_BUTTON_RIGHT] & 2) == 2){
                    auto diff_x = mouse->x - mouse->prev_x;
                    auto diff_y = mouse->y - mouse->prev_y;
                    mouse->prev_x = mouse->x;
                    mouse->prev_y = mouse->y;

                    //t.local.rot.x += diff_x * c.rotate_speed;
                    //t.local.rot.y += diff_y * c.rotate_speed;

                    t.euler_rot.x += diff_x * c.rotate_speed;
                    t.euler_rot.y += diff_y * c.rotate_speed;
                }  
                if(mouse->prev_scroll != mouse->scroll){
                    auto diff_z = mouse->scroll - mouse->prev_scroll;
                    mouse->prev_scroll = mouse->scroll;

                    t.local.pos.z += diff_z * c.move_speed;
                }          
            }
        }
    }
}
