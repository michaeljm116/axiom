#include "pch.h"
#include <flecs.h>
#include <optick.h>

#include "sys-transform.h"
#include "sys-log.h"
#include "sys-timer.h"

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


	flecs::world world;
	world.set<axiom::Cmp_LogFile>(axiom::Cmp_LogFile("log.txt"));


	for(int i = 0; i < 1000; ++i){
		if(i == 10){
			world.set<axiom::Cmp_Log>({axiom::LogLevel::INFO, false, "Hello world i = 10"});
		}
		world.progress();
	}
	world.remove<axiom::Cmp_LogFile>();
	return 0;
	
};