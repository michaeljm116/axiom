#include "pch.h"
#include <flecs-world.h>
#include <optick.h>
#include <taskflow/taskflow.hpp>

#include "sys-transform.h"
#include "sys-log.h"
#include "sys-timer.h"
#include "sys-resource.h"
#include "sys-window.h"

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
#include <sstream>

using namespace axiom;

int main(){
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
	version = volkGetInstanceVersion();
	std::stringstream ss;
	ss << "Vulkan Version: " << VK_VERSION_MAJOR(version) << "."
		<< VK_VERSION_MINOR(version) << "." << VK_VERSION_PATCH(version);
	std::string info = ss.str();

#pragma endregion Initializing volk

	tf::Executor executor;
	tf::Taskflow taskflow;

	axiom::Sys_Logger logger;
	axiom::Sys_Timer timer;
	axiom::Sys_Resource resource;
	axiom::Sys_Window window("Axiom Engine", 1280, 720);

	g_world.add<axiom::Cmp_CurrentTime>();
	g_world.add<axiom::Cmp_LogFile>();
	g_world.add<axiom::Cmp_Timer>();
	
	std::string assets_folder = "../../assets/";
	auto t = g_world.entity("Models Timer");
	auto at = g_world.entity("Animations TImer");


	//Load Models
	t.add<axiom::Cmp_Timer>();
	resource.LoadDirectory(assets_folder + "Models");
	t.remove<axiom::Cmp_Timer>();

	//Load Animations
	at.add<axiom::Cmp_Timer>();
	resource.LoadDirectory(assets_folder + "Animations");
	at.remove<axiom::Cmp_Timer>();

	//Load Materials
	resource.LoadMaterials(assets_folder + "Materials.xml");


	auto e = g_world.lookup("A_Primitive_Helix_01.pm");
	auto m = e.get<axiom::Cmp_Res_Model>();
	auto f = e.get<axiom::Cmp_Resource>();

	auto bird = g_world.lookup("Bird.anim");
	auto b = bird.get<axiom::Cmp_Res_Animations>();
	auto bb = bird.get<axiom::Cmp_Resource>();

	auto gold = g_world.lookup("Gold");
	auto g = gold.get<axiom::Cmp_Res_Material>();
	auto gg = gold.get<axiom::Cmp_Resource>();


	auto e_froku = g_world.entity("Froku");
	auto r_froku = g_world.lookup("froku2.pm");
	auto d_froku = r_froku.get<axiom::Cmp_Res_Model>()->data;

std::vector<flecs::entity> body_parts;
for(auto d : d_froku.meshes){
    std::string name = d.name;
    auto bp = g_world.entity(name.c_str());
    bp.set<axiom::R_Mesh>({d});
    bp.child_of(e_froku);
    body_parts.push_back(bp);
}

for(auto bp : body_parts){
    std::cout << bp.name() << std::endl;
}
auto head = g_world.lookup("Froku::head");
auto h_cmp = head.get<axiom::R_Mesh>();
auto afro = e_froku.lookup("afro");
afro.child_of(head);
auto a_cmp = afro.get<axiom::R_Mesh>();

auto prnt = afro.parent();
auto prnt_cmp = prnt.get<axiom::Cmp_Res_Model>();

auto* twindow = g_world.get<Cmp_Window>()->window;

while(!glfwWindowShouldClose(twindow)){
	g_world.progress();
}
	
};