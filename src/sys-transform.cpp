#include "pch.h"
#include "sys-transform.h"

axiom::Sys_StaticTransformSystem::Sys_StaticTransformSystem(flecs::world &world)
{
    /*world.system<const Cmp_Transform>("StaticTransformSystem")
        .kind(flecs::OnSet)
        .iter([this](flecs::iter& it, flecs::column<const Cmp_Transform> t){
            update(it, t);
        });*/
    world.system<Cmp_Transform>("StaticTransformSystem")
        .kind(flecs::OnUpdate)
        .each([this](Cmp_Transform& t){
            update(t);
        });
}

axiom::Sys_StaticTransformSystem::~Sys_StaticTransformSystem()
{
}

void axiom::Sys_StaticTransformSystem::initialize()
{
}

void axiom::Sys_StaticTransformSystem::update(Cmp_Transform &t)
{
    t.local.pos.x += 1;
    std::cout << "\nPos: " << t.local.pos.x;
}

axiom::Sys_Transform::Sys_Transform(flecs::world &world)
{
}

axiom::Sys_Transform::~Sys_Transform()
{
}

void axiom::Sys_Transform::initialize()
{
}
