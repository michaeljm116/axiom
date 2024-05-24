
#include "sys-shader.h"
#include <fstream>
#include <iostream>
#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/resource_limits_c.h>
#include <glslang/Include/ResourceLimits.h>
#include <SPIRV/GlslangToSpv.h>
#include "flecs-world.h"
#include "sys-log.h"


namespace Axiom{
    namespace Render{
        namespace Shader{

            const TBuiltInResource DefaultTBuiltInResource = {
            /* .MaxLights = */ 32,
            /* .MaxClipPlanes = */ 6,
            /* .MaxTextureUnits = */ 32,
            /* .MaxTextureCoords = */ 32,
            /* .MaxVertexAttribs = */ 64,
            /* .MaxVertexUniformComponents = */ 4096,
            /* .MaxVaryingFloats = */ 64,
            /* .MaxVertexTextureImageUnits = */ 32,
            /* .MaxCombinedTextureImageUnits = */ 80,
            /* .MaxTextureImageUnits = */ 32,
            /* .MaxFragmentUniformComponents = */ 4096,
            /* .MaxDrawBuffers = */ 32,
            /* .MaxVertexUniformVectors = */ 128,
            /* .MaxVaryingVectors = */ 8,
            /* .MaxFragmentUniformVectors = */ 16,
            /* .MaxVertexOutputVectors = */ 16,
            /* .MaxFragmentInputVectors = */ 15,
            /* .MinProgramTexelOffset = */ -8,
            /* .MaxProgramTexelOffset = */ 7,
            /* .MaxClipDistances = */ 8,
            /* .MaxComputeWorkGroupCountX = */ 65535,
            /* .MaxComputeWorkGroupCountY = */ 65535,
            /* .MaxComputeWorkGroupCountZ = */ 65535,
            /* .MaxComputeWorkGroupSizeX = */ 1024,
            /* .MaxComputeWorkGroupSizeY = */ 1024,
            /* .MaxComputeWorkGroupSizeZ = */ 64,
            /* .MaxComputeUniformComponents = */ 1024,
            /* .MaxComputeTextureImageUnits = */ 16,
            /* .MaxComputeImageUniforms = */ 8,
            /* .MaxComputeAtomicCounters = */ 8,
            /* .MaxComputeAtomicCounterBuffers = */ 1,
            /* .MaxVaryingComponents = */ 60,
            /* .MaxVertexOutputComponents = */ 64,
            /* .MaxGeometryInputComponents = */ 64,
            /* .MaxGeometryOutputComponents = */ 128,
            /* .MaxFragmentInputComponents = */ 128,
            /* .MaxImageUnits = */ 8,
            /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
            /* .MaxCombinedShaderOutputResources = */ 8,
            /* .MaxImageSamples = */ 0,
            /* .MaxVertexImageUniforms = */ 0,
            /* .MaxTessControlImageUniforms = */ 0,
            /* .MaxTessEvaluationImageUniforms = */ 0,
            /* .MaxGeometryImageUniforms = */ 0,
            /* .MaxFragmentImageUniforms = */ 8,
            /* .MaxCombinedImageUniforms = */ 8,
            /* .MaxGeometryTextureImageUnits = */ 16,
            /* .MaxGeometryOutputVertices = */ 256,
            /* .MaxGeometryTotalOutputComponents = */ 1024,
            /* .MaxGeometryUniformComponents = */ 1024,
            /* .MaxGeometryVaryingComponents = */ 64,
            /* .MaxTessControlInputComponents = */ 128,
            /* .MaxTessControlOutputComponents = */ 128,
            /* .MaxTessControlTextureImageUnits = */ 16,
            /* .MaxTessControlUniformComponents = */ 1024,
            /* .MaxTessControlTotalOutputComponents = */ 4096,
            /* .MaxTessEvaluationInputComponents = */ 128,
            /* .MaxTessEvaluationOutputComponents = */ 128,
            /* .MaxTessEvaluationTextureImageUnits = */ 16,
            /* .MaxTessEvaluationUniformComponents = */ 1024,
            /* .MaxTessPatchComponents = */ 120,
            /* .MaxPatchVertices = */ 32,
            /* .MaxTessGenLevel = */ 64,
            /* .MaxViewports = */ 16,
            /* .MaxVertexAtomicCounters = */ 0,
            /* .MaxTessControlAtomicCounters = */ 0,
            /* .MaxTessEvaluationAtomicCounters = */ 0,
            /* .MaxGeometryAtomicCounters = */ 0,
            /* .MaxFragmentAtomicCounters = */ 8,
            /* .MaxCombinedAtomicCounters = */ 8,
            /* .MaxAtomicCounterBindings = */ 1,
            /* .MaxVertexAtomicCounterBuffers = */ 0,
            /* .MaxTessControlAtomicCounterBuffers = */ 0,
            /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
            /* .MaxGeometryAtomicCounterBuffers = */ 0,
            /* .MaxFragmentAtomicCounterBuffers = */ 1,
            /* .MaxCombinedAtomicCounterBuffers = */ 1,
            /* .MaxAtomicCounterBufferSize = */ 16384,
            /* .MaxTransformFeedbackBuffers = */ 4,
            /* .MaxTransformFeedbackInterleavedComponents = */ 64,
            /* .MaxCullDistances = */ 8,
            /* .MaxCombinedClipAndCullDistances = */ 8,
            /* .MaxSamples = */ 4,
            /* .maxMeshOutputVerticesNV = */ 256,
            /* .maxMeshOutputPrimitivesNV = */ 512,
            /* .maxMeshWorkGroupSizeX_NV = */ 32,
            /* .maxMeshWorkGroupSizeY_NV = */ 1,
            /* .maxMeshWorkGroupSizeZ_NV = */ 1,
            /* .maxTaskWorkGroupSizeX_NV = */ 32,
            /* .maxTaskWorkGroupSizeY_NV = */ 1,
            /* .maxTaskWorkGroupSizeZ_NV = */ 1,
            /* .maxMeshViewCountNV = */ 4,
            /* .maxMeshOutputVerticesEXT = */ 256,
            /* .maxMeshOutputPrimitivesEXT = */ 256,
            /* .maxMeshWorkGroupSizeX_EXT = */ 128,
            /* .maxMeshWorkGroupSizeY_EXT = */ 128,
            /* .maxMeshWorkGroupSizeZ_EXT = */ 128,
            /* .maxTaskWorkGroupSizeX_EXT = */ 128,
            /* .maxTaskWorkGroupSizeY_EXT = */ 128,
            /* .maxTaskWorkGroupSizeZ_EXT = */ 128,
            /* .maxMeshViewCountEXT = */ 4,
            /* .maxDualSourceDrawBuffersEXT = */ 1,

            /* .limits = */ {
                /* .nonInductiveForLoops = */ 1,
                /* .whileLoops = */ 1,
                /* .doWhileLoops = */ 1,
                /* .generalUniformIndexing = */ 1,
                /* .generalAttributeMatrixVectorIndexing = */ 1,
                /* .generalVaryingIndexing = */ 1,
                /* .generalSamplerIndexing = */ 1,
                /* .generalVariableIndexing = */ 1,
                /* .generalConstantMatrixVectorIndexing = */ 1,
            }};


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

                const TBuiltInResource* Resources = &DefaultTBuiltInResource;
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
                glslang::SpvOptions ops = {
                    .validate = true
                };
                glslang::GlslangToSpv(*program.getIntermediate(shader_type), out_spirv, &ops);
                return out_spirv;
            }
        }
    }
}