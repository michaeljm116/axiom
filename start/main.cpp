#include "pch.h"
#include <flecs-world.h>
#include <optick.h>
#include <taskflow/taskflow.hpp>
#include <sstream>

#include "sys-transform.h"
#include "sys-log.h"
#include "sys-timer.h"
#include "sys-resource.h"
#include "sys-window.h"
#include "sys-serialize.h"
#include "sys-scene.h"
#include "sys-bvh.h"

/* Set platform defines at build time for volk to pick up. */
#if defined(_WIN32)
#   define VK_USE_PLATFORM_WIN32_KHR
#elif defined(__linux__) || defined(__unix__)
#   define VK_USE_PLATFORM_XLIB_KHR
#elif defined(__APPLE__)
#   define VK_USE_PLATFORM_MACOS_MVK
#else
#   error "Platform not supported by this example."
#endif
#define VOLK_IMPLEMENTATION
#include <volk.h>
#include "texture.h"
#include "render-base.h"
#include "sys-compute-raytracer.h"
#include "sys-hardware-rasterizer.h"

using namespace Axiom;

int main(){	
	Axiom::Log::initialize();

	Axiom::Timer::initialize();
	Axiom::Resource::initialize();
	Axiom::Window::init("Axiom Engine", 1280, 720);


	g_world.add<Axiom::Cmp_CurrentTime>();
	g_world.add<Axiom::Cmp_LogFile>();
	g_world.add<Axiom::Cmp_Timer>();



#pragma region Volk Init
	VkResult r;
	uint32_t version;
	void* ptr;
	
	ptr = 
#if defined(_WIN32)
    &vkCreateWin32SurfaceKHR;
#elif defined(__linux__) || defined(__unix__)
    &vkCreateXlibSurfaceKHR;
#elif defined(__APPLE__)
    &vkCreateMacOSSurfaceMVK;
#else
    /* Platform not recogized for testing. */
    NULL;
#endif
	r = volkInitialize();
	Log::check(r == VK_SUCCESS, "Initializing Volk");
	version = volkGetInstanceVersion();
	std::stringstream ss;
	ss << "Vulkan Version: " << VK_VERSION_MAJOR(version) << "."
		<< VK_VERSION_MINOR(version) << "." << VK_VERSION_PATCH(version);
	std::string info = ss.str();

#pragma endregion Initializing volk

	//const std::string assets_folder = "../../assets/";
	g_world.add<Axiom::Resource::Cmp_Directory>();
	auto& assets_folder = g_world.get_ref<Axiom::Resource::Cmp_Directory>().get()->assets;
	assets_folder = "../../assets/";

	Axiom::Resource::load_directory(assets_folder + "Models/PrincipiaModels");
	Axiom::Resource::load_directory(assets_folder + "Animations");
	Axiom::Resource::load_materials(assets_folder + "Materials.xml");


	auto suzanne = g_world.entity("Suzanne");
	auto sponza = g_world.entity("Sponza");
	Cmp_Resource suzanne_res = {.file_path = assets_folder + "Models/GLTF/Suzanne/glTF", .file_name = "Suzanne.gltf"};
	Cmp_Resource sponza_res = {.file_path = assets_folder + "Models/GLTF/Sponza/glTF", .file_name = "Sponza.gltf"};
	suzanne.set(suzanne_res);
	suzanne.set(Cmp_AssimpModel());
	sponza.set(sponza_res);
	sponza.set(Cmp_AssimpModel());

	auto* twindow = g_world.get<Cmp_Window>()->window;
	Axiom::Transform::initialize();
	g_world.add<Axiom::Cmp_Serialize>();
	scene::initialize();	
	g_world.set<Axiom::Cmp_Scene>({assets_folder + "Scenes/", "TestEntrance.xml", 0});
	g_world.add<Axiom::Render::Cmp_Vulkan>();

	auto render_type = Axiom::Render::RendererType::kHardwareRasterizer;
	//auto render_type = Axiom::Render::RendererType::kComputeRaytracer;

	switch (render_type)
	{
	case Axiom::Render::RendererType::kComputeRaytracer:
		Axiom::Bvh::Init();
		g_world.add<Axiom::Render::Cmp_ComputeRaytracer>();
		Axiom::Render::Compute::initialize_raytracing();
		break;
	case Axiom::Render::RendererType::kHardwareRasterizer:
		g_world.add<Axiom::Render::Cmp_HardwareRaster>();
		g_world.add<Axiom::Render::Cmp_GraphicsPipeline>();
		Axiom::Render::Hardware::initialize_raster();
		break;
	default:
		break;
	}
	
	g_world.progress();

	while(!glfwWindowShouldClose(twindow)){
		if(render_type == Axiom::Render::RendererType::kComputeRaytracer){
			Axiom::Bvh::Build();
			auto bvh = Axiom::Bvh::bvh_comp;

			Axiom::Render::Compute::g_raytracer.update_bvh(bvh->prims, bvh->root, bvh->num_nodes);
			uint32_t ii;
			Render::Compute::g_raytracer.start_frame(ii);
			g_world.lookup("Update Renderer");		
			Render::Compute::g_raytracer.end_frame(ii);
		}
		else{

			uint32_t ii = 0;
			//Render::Hardware::g_raster.start_frame(ii);
			//Render::Hardware::g_raster.end_frame(ii);
			//Render::Hardware::g_raster.draw_frame();
		}
		g_world.progress();
	}
	Axiom::Window::destruct();
};