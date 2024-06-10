#pragma once
namespace Axiom
{
    namespace Scene
    {
        struct Cmp_Camera
        {
            float fov = 70.f;
            float near = 1.f; 
            float far = 1000.f;

            float rotate_speed = 1.f;
            float move_speed = 5.f;
        };
            
    }
} // namespace Axiom
