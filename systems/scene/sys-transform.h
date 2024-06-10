#include "../components/scene/cmp-transform.h"
#include <flecs-world.h>

namespace Axiom
{
//flecs::system SYS_Trans = flecs::world::system<CMP_Transform>("Transform System");
    namespace Transform
    {
        void initialize();
        void transform_static_entity(flecs::entity e, Cmp_Transform& t, Cmp_Static& s);
        void transform_dynamic_entity(flecs::entity e, Cmp_Transform& t, Cmp_Dynamic& d);
        void transform_entity(flecs::entity e); 
        glm::vec3 rotate_aabb( const glm::mat3& m);     

        void force_transform(Cmp_Transform& t);
    };

}