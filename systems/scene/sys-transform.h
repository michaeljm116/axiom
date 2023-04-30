#include "../components/scene/cmp-transform.h"
#include <flecs-world.h>

namespace axiom
{
//flecs::system SYS_Trans = flecs::world::system<CMP_Transform>("Transform System");
    namespace transform
    {
        void Init();
        void Static_Transform(flecs::entity e, Cmp_Transform& t, Cmp_Static& s);
        void Dynamic_Transform(flecs::entity e, Cmp_Transform& t, Cmp_Dynamic& d);
        void recursive_transform(flecs::entity e);
        glm::vec3 rotate_aabb( const glm::mat3& m);     

    };

}