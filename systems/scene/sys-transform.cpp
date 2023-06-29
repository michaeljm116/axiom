#include "pch.h"
#include "sys-transform.h"

namespace Axiom
{
    namespace Transform
    {
        void Init()
        {
            /*
            world.observer<Cmp_Transform*, Cmp_Static>("StaticTransformSystem")
            .event(flecs::OnSet)
            .each([this](flecs::entity e, Cmp_Transform& t, Cmp_Static& s) {
                this->Static_Transform(e,t,s);
            });

            world.system<Cmp_Transform*, Cmp_Dynamic>("DynamicTransformSystem")
            .kind(flecs::OnUpdate)
            .term_at(2).parent().cascade()
            .each([this](flecs::entity e, Cmp_Transform& t, Cmp_Dynamic& d){
                this->Dynamic_Transform(e,t,d);
            });*/

            auto static_transform = g_world.query_builder<const Cmp_Transform, Cmp_Transform>()
                .term_at(1).second<Cmp_Static>()
                .term_at(2).second<Cmp_Static>()
                .term_at(2).parent().cascade().optional()
                .build();
            
            /*auto dynamic_transform = g_world.query_builder<const Cmp_Transform, Cmp_Transform>()
                .term_at(1).second<Cmp_Dynamic>()
                .term_at(2).second<Cmp_Dynamic>()
                .term_at(2).parent().cascade().optional()
                .build();*/

            g_world.observer<const Cmp_Transform, Cmp_Transform>()
            .term_at(1).parent()
            .event(flecs::OnSet)
            .event(flecs::OnAdd)
            .iter([](flecs::iter& it, const Cmp_Transform * parent, Cmp_Transform* child){
                                //build rotaiton matrix
                glm::mat4 rot_m = glm::rotate(glm::radians(child->euler_rot.x), glm::vec3(1.f, 0.f, 0.f));
                rot_m = glm::rotate(rot_m, glm::radians(child->euler_rot.y), glm::vec3(0.f, 1.f, 0.f));
                rot_m = glm::rotate(rot_m, glm::radians(child->euler_rot.z), glm::vec3(0.f, 0.f, 1.f));
                child->local.rot = rot_m;
                child->global.rot *= child->local.rot;

                //build position and scale matrix.
                glm::mat4 pos_m = glm::translate(glm::vec3(child->local.pos));
                glm::mat4 sca_m = glm::scale(glm::vec3(child->local.sca));
                glm::mat4 local = pos_m * rot_m;
 
                //combine them into 1 and multiply by parent if u haz parent
                if(parent){
                    child->global.sca = child->local.sca * parent->global.sca;
                    child->trm = parent->world * local;
                    local *= sca_m;
                    child->world = parent->world * local;
                }
                else{ //There's no parent 
                    child->global.sca = child->local.sca;
                    child->trm = local;
                    local *= sca_m;
                    child->world = local;
                }
                            
            });

            // g_world.system<Cmp_Transform, Cmp_Dynamic>()
            // .term_at(1).parent()
            // .kind(flecs::OnUpdate)
            // .iter([](flecs::iter& it, const Cmp_Transform * parent, Cmp_Transform* child){

            // });

            /*static_transform.iter([](flecs::iter& it, const Cmp_Transform *parent, Cmp_Transform* child)
            {
                //build rotaiton matrix
                glm::mat4 rot_m = glm::rotate(glm::radians(child->euler_rot.x), glm::vec3(1.f, 0.f, 0.f));
                rot_m = glm::rotate(rot_m, glm::radians(child->euler_rot.y), glm::vec3(0.f, 1.f, 0.f));
                rot_m = glm::rotate(rot_m, glm::radians(child->euler_rot.z), glm::vec3(0.f, 0.f, 1.f));
                child->local.rot = rot_m;
                child->global.rot *= child->local.rot;

                //build position and scale matrix.
                glm::mat4 pos_m = glm::translate(glm::vec3(child->local.pos));
                glm::mat4 sca_m = glm::scale(glm::vec3(child->local.sca));
                glm::mat4 local = pos_m * rot_m;

                //combine them into 1 and multiply by parent if u haz parent
                if(parent){
                    child->global.sca = child->local.sca * parent->global.sca;
                    child->trm = parent->world * local;
                    local *= sca_m;
                    child->world = parent->world * local;
                }
                else{ //There's no parent 
                    child->global.sca = child->local.sca;
                    child->trm = local;
                    local *= sca_m;
                    child->world = local;
                }
            });*/

            //g_world.observer<Cmp_Transform, Cmp_Static>();
            
            
        }
        

        void Static_Transform(flecs::entity e, Cmp_Transform &t, Cmp_Static &s)
        {
            auto q = g_world.query_builder<Cmp_Transform, Cmp_Transform>().term_at(2).parent().cascade()
            .build();

            auto traverse_children = [&](flecs::entity e, const auto& self){
                Cmp_Transform* tc = e.get_mut<Cmp_Transform>();
                bool has_parent = e.parent().is_entity();

                //build rotaiton matrix
                glm::mat4 rot_m = glm::rotate(glm::radians(tc->euler_rot.x), glm::vec3(1.f, 0.f, 0.f));
                rot_m = glm::rotate(rot_m, glm::radians(tc->euler_rot.y), glm::vec3(0.f, 1.f, 0.f));
                rot_m = glm::rotate(rot_m, glm::radians(tc->euler_rot.z), glm::vec3(0.f, 0.f, 1.f));
                tc->local.rot = rot_m;
                tc->global.rot *= tc->local.rot;

                //build position and scale matrix.
                glm::mat4 pos_m = glm::translate(glm::vec3(tc->local.pos));
                glm::mat4 sca_m = glm::scale(glm::vec3(tc->local.sca));
                glm::mat4 local = pos_m * rot_m;

                //combine them into 1 and multiply by parent if u haz parent
                if(has_parent){
                    const auto* parent = e.parent().get<Cmp_Transform>();
                    tc->global.sca = tc->local.sca * parent->global.sca;
                    tc->trm = parent->world * local;
                    local *= sca_m;
                    tc->world = parent->world * local;
                }
                else{ //There's no parent 
                    tc->global.sca = tc->local.sca;
                    tc->trm = local;
                    local *= sca_m;
                    tc->world = local;
                }

                //e.children([&](flecs::entity child){ self(child, self); });
            };
            traverse_children(e, traverse_children);
        }

        void Dynamic_Transform(flecs::entity e, Cmp_Transform &t, Cmp_Dynamic &d)
        {
        }

        void recursive_transform(flecs::entity e)
        {

        }

        glm::vec3 rotate_aabb(const glm::mat3 &m)
        {
            //set up cube
            glm::vec3 extents = glm::vec3(1);
            glm::vec3 v[8];
            v[0] = extents;
            v[1] = glm::vec3(extents.x, extents.y, -extents.z);
            v[2] = glm::vec3(extents.x, -extents.y, -extents.z);
            v[3] = glm::vec3(extents.x, -extents.y, extents.z);
            v[4] = glm::vec3(-extents);
            v[5] = glm::vec3(-extents.x, -extents.y, extents.z);
            v[6] = glm::vec3(-extents.x, extents.y, -extents.z);
            v[7] = glm::vec3(-extents.x, extents.y, extents.z);

            //transform them
            //#pragma omp parallel for
            for (int i = 0; i < 8; ++i) {
                v[i] = abs(m * v[i]);// glm::vec4(v[i], 1.f));

            }

            //compare them
            glm::vec3 vmax = glm::vec3(FLT_MIN);
            for (int i = 0; i < 8; ++i) {
                vmax.x = std::max(vmax.x, v[i].x);
                vmax.y = std::max(vmax.y, v[i].y);
                vmax.z = std::max(vmax.z, v[i].z);
            }

            return vmax;
        }
    }
}
    