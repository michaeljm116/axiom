
#pragma once
#include "cmp-shader.h"
#include <string>
#include <vector>
#include <optional>

namespace Axiom{
    namespace Render{
        namespace Shader{
            void init();
            void finalize(); 
            std::optional<std::vector<uint32_t>> compile_glsl(const std::string& file_name, Shader::Type type);

            
        }
    }
}