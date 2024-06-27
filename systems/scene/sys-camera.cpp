#include "sys-camera.h"
#include "flecs-world.h"
#include "cmp-window.h"
#include "sys-transform.h"

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
                auto* kb = g_world.get_mut<Cmp_Keyboard>();
                const auto key_is_down = [](int key){
                    return (key & 2) == 2;
                };
                glm::vec4 velocity = glm::vec4(0.f);
                if(key_is_down(kb->keys[GLFW_KEY_D])) velocity.x -= c.move_speed_kb;
                if(key_is_down(kb->keys[GLFW_KEY_A])) velocity.x += c.move_speed_kb;
                if(key_is_down(kb->keys[GLFW_KEY_SPACE])) velocity.y -= c.move_speed_kb;
                if(key_is_down(kb->keys[GLFW_KEY_LEFT_ALT])) velocity.y += c.move_speed_kb;
                if(key_is_down(kb->keys[GLFW_KEY_S])) velocity.z -= c.move_speed_kb;
                if(key_is_down(kb->keys[GLFW_KEY_W])) velocity.z += c.move_speed_kb;
                if(velocity != glm::vec4(0)){
                    auto pos = velocity * t.trm;
                    t.local.pos += pos;
                }

                if((mouse->buttons[GLFW_MOUSE_BUTTON_RIGHT] & 2) == 2){
                    auto diff_x = mouse->x - mouse->prev_x;
                    auto diff_y = mouse->y - mouse->prev_y;
                    mouse->prev_x = mouse->x;
                    mouse->prev_y = mouse->y;

                    t.euler_rot.x += diff_y * c.rotate_speed;
                    t.euler_rot.y += diff_x * c.rotate_speed;
                }  
                if(mouse->prev_scroll != mouse->scroll){
                    auto diff_z = mouse->scroll - mouse->prev_scroll;
                    mouse->prev_scroll = mouse->scroll;
                    auto pos = glm::vec4(0, 0, diff_z * c.move_speed_mouse, 0) * t.trm;
                    
                    t.local.pos += pos;
                }     
                Transform::force_transform(t);
            }
        }
    }
}
