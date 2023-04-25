#include "pch.h"
#include <flecs.h>
#include <optick.h>
#include <taskflow/taskflow.hpp>

#include "sys-transform.h"
#include "sys-log.h"
#include "sys-timer.h"
#include "sys-resource.h"

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

	flecs::world world;
	axiom::Sys_Logger logger(world);
	axiom::Sys_Timer timer(world);
	axiom::Sys_Resource resource(world);

	world.add<axiom::Cmp_CurrentTime>();
	world.add<axiom::Cmp_LogFile>();
	world.add<axiom::Cmp_Timer>();
	
	std::string assets_folder = "../../assets/";

	auto t = world.entity("Models Timer");
	t.add<axiom::Cmp_Timer>();
	resource.LoadDirectory(assets_folder + "Models");
	t.remove<axiom::Cmp_Timer>();
	auto at = world.entity("Animations TImer");
	at.add<axiom::Cmp_Timer>();
	resource.LoadDirectory(assets_folder + "Animations");
	at.remove<axiom::Cmp_Timer>();
	auto e = world.lookup("A_Primitive_Helix_01.pm");
	auto m = e.get<axiom::Cmp_Res_Model>();
	auto f = e.get<axiom::Cmp_Resource>();

	auto bird = world.lookup("Bird.anim");
	auto b = bird.get<axiom::Cmp_Res_Animations>();
	auto bb = bird.get<axiom::Cmp_Resource>();
	return 0;
	
};