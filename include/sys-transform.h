#include "cmp-transform.h"
#include <flecs.h>

namespace axiom
{
class Sys_StaticTransformSystem{
    public:
        Sys_StaticTransformSystem(flecs::world& world);
        ~Sys_StaticTransformSystem();
        void initialize();
    private:
        void update(Cmp_Transform& t);
};


//flecs::system SYS_Trans = flecs::world::system<CMP_Transform>("Transform System");
class Sys_Transform{
    public:
    Sys_Transform(flecs::world& world);
    ~Sys_Transform();

    void initialize();

    private:
    //void update(flecs::iter& it, flecs::column<)
};

}