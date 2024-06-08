#pragma once
namespace Axiom
{
    namespace Scene
    {
        struct Cmp_Camera
        {
            float fov = 1000.f;
            float near = 1.f; 
            float far = 70.f;

            float rotate_speed = 5.f;
            float move_speed = 1.f;
        };
            
    }
} // namespace Axiom
