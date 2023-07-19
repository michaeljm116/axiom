#pragma once

namespace Axiom
{
    namespace Render
    {
        struct Cmp_Material
        {
            int id;
            int unique_id;

            Cmp_Material(){};
            Cmp_Material(int i) {id = i;};
            Cmp_Material(int i, int ui) { id = i; unique_id = ui;}
        };
    }
    
} // namespace Axiom
