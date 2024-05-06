
#include "sys-shader.h"
#include <fstream>
#include <iostream>
#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <SPIRV/GlslangToSpv.h>
#include "flecs-world.h"
#include "sys-log.h"


namespace Axiom{
    namespace Render{
        namespace Shader{

            auto get_shader_type = [](Type shaderType) 
            {
                switch (shaderType) {
                    case eVertex:
                        return EShLangVertex;           // Vertex shader
                    case eFragment:
                        return EShLangFragment;         // Fragment shader
                    case eGeometry:
                        return EShLangGeometry;         // Geometry shader
                    case eTesselation:
                        return EShLangTessControl;      // Tesselation control shader
                    case eCompute:
                        return EShLangCompute;          // Compute shader
                    case eHull:
                        return EShLangTessEvaluation;   // Tesselation evaluation shader (often called Hull shader)
                    case eMesh:
                        return EShLangMeshNV;           // Mesh shader, assuming NV extension
                    default:
                        throw std::invalid_argument("Unsupported shader type");
                }
            };

            auto read_file = [](const std::string& filename) -> std::string {
                std::ifstream file(filename, std::ios::ate | std::ios::binary);
                if (!file.is_open()) {
                    throw std::runtime_error("failed to open file: " + filename);
                }

                size_t fileSize = (size_t)file.tellg();
                std::vector<char> buffer(fileSize);

                file.seekg(0);
                file.read(buffer.data(), fileSize);
                file.close();

                // Use the buffer to construct a string and return it
                return std::string(buffer.begin(), buffer.end());
            };


            void init()
            {
                glslang::InitializeProcess();
            }
            void finalize()
            {
                glslang::FinalizeProcess();
            }
            std::optional<std::vector<uint32_t>>  compile_glsl(const std::string &file_name, Shader::Type type)
            {
                auto shader_code = read_file(file_name);
                const char* shader_code_cstr = shader_code.c_str();
                auto shader_type = get_shader_type(type);

                glslang::TShader shader(shader_type);
                shader.setStrings(&shader_code_cstr, 1);

                // Set up default version and profile
                int clientInputSemanticsVersion = 100; // Maps to Vulkan 1.0 semantics
                glslang::EShTargetClientVersion VulkanClientVersion = glslang::EShTargetVulkan_1_0;
                glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_0;

                shader.setEnvInput(glslang::EShSourceGlsl, shader_type, glslang::EShClientVulkan, clientInputSemanticsVersion);
                shader.setEnvClient(glslang::EShClientVulkan, VulkanClientVersion);
                shader.setEnvTarget(glslang::EShTargetSpv, TargetVersion);

                auto Resources = GetDefaultResources(); // Use the default built-in resources
                // Parse and compile the shader
                EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
                if (!shader.parse(Resources, 100, false, messages)) {
                    std::string error_info = shader.getInfoLog();
                    std::cerr << "GLSL Parsing Failed for: " << shader.getInfoLog() << std::endl;
                    Axiom::Log::send(Log::Level::ERROR, "GLSL Parsing Failed for: " + error_info);
                    return std::nullopt;
                }

                // Link the shader into a program
                glslang::TProgram program;
                program.addShader(&shader);
                if (!program.link(messages)) {
                    std::string error_info = program.getInfoLog();
                    std::cerr << "Shader Program Linking Failed: " << program.getInfoLog() << std::endl;
                    Axiom::Log::send(Log::Level::ERROR, "Shader Program Linking Failed: " + error_info);
                    return std::nullopt;
                }

                // Convert to SPIRV
                std::vector<uint32_t> out_spirv;
                glslang::SpvOptions ops;
                glslang::GlslangToSpv(*program.getIntermediate(shader_type), out_spirv, &ops);
                return out_spirv;
            }
        }
    }
}