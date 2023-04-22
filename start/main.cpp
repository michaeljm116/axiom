#include "pch.h"
#include <flecs.h>
#include <glm/glm.hpp>
#include "helpers.h"
#include "sys-movement.h"
#include "core/render/window.h"
#include "core/util/log.h"
#include "opengl-tutorial.h"
#include <optick.h>

#include "cmp-transform.h"
#include "sys-transform.h"

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

void FlecsTutorial();
void randoish();

int main(){
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
	axiom::Check(r == VK_SUCCESS, "Initializing Volk");
	version = volkGetInstanceVersion();
	std::stringstream ss;
	ss << "Vulkan Version: " << VK_VERSION_MAJOR(version) << "."
		<< VK_VERSION_MINOR(version) << "." << VK_VERSION_PATCH(version);
	std::string info = ss.str();
	axiom::LogInfo(info);

	return 0;
	
};

void randoish(){
	flecs::world world;
	axiom::Sys_StaticTransformSystem sys_static_transform(world);
	auto e = world.entity("asdf");
	e.set<axiom::Cmp_Transform>(axiom::Cmp_Transform());


	for(int i = 0; i < 1000; ++i){
		world.progress();
		auto* t = e.get_mut<axiom::Cmp_Transform>();
		t->local.pos.x += 1;
		e.modified<axiom::Cmp_Transform>();
	}
	std::cout << "Hello";
	system("Pause");


	OPTICK_THREAD("MainThread");
	OPTICK_START_CAPTURE();

	OPTICK_PUSH("Pass1");
	std::this_thread::sleep_for(
	std::chrono::milliseconds(2));

	axiom::SWindow().Init();
	axiom::OpenGLTutorial openglTutorial(axiom::GetWindow());
	openglTutorial.Init();

	OPTICK_POP();
	while(!glfwWindowShouldClose(axiom::GetWindow())){
		OPTICK_PUSH("Render Thread");
		openglTutorial.Render();
		if(axiom::SWindow().GetKey() == GLFW_KEY_P)
			openglTutorial.ScreenShot();
		glfwPollEvents();
		OPTICK_POP();
	}

	OPTICK_STOP_CAPTURE();
	OPTICK_SAVE_CAPTURE("profiler_dump");
};

void FlecsTutorial(){
	// Worlds do everything like store entities and their components
	// does queries and runs systems
	// might be good to make it global if possible
	flecs::world world;

	// Entities are entities, they each have a unique id
	// 64 bits, 4billy allowed. can check if alive o nah
	flecs::entity e = world.entity();
	e.is_alive(); //currently true
	e.destruct();
	e.is_alive(); //currently false

	//You can name entities!!!
	flecs::entity me = world.entity("Mike");
	std::cout << "Entity name: " << me.name() << std::endl;
	flecs::entity me2 = world.lookup("Mike");
	std::cout << "Looked up: " << me2.name() << std::endl;

	//Add the velocity Component
	me.add<Velocity>();
	//woah it adds a component if its not already added!
	me.set<Position>({10,20}).set<Velocity>({1,2});

	//Get a component
	auto* pos = me.get<Position>();

	//Remove a component
	me.remove<Position>();

	world.set_threads(4);
	auto sss = world.system<Position, const Velocity>();
	auto tick_source = world.timer().interval(1.0);
	sss.tick_source(tick_source);
	tick_source.stop();
	tick_source.start();

	auto body = world.entity("Body");
	auto legs = world.entity("Legs");
	legs.child_of(body);
	auto person = world.prefab("Person");
	body.is_a(person);

	

	//i haven't comprehended what this does yet but...
	//flecs::entity pos_e = world.id<Position>();	
	//std::cout << "Name: " << pos_e.name() << std::endl;


	//tbh still not sure what a component on this is liek what does this do?
	//const EcsComponent *c = me.get<flecs::Component>();
	//std::cout << "Component size: " << c->size << std::endl;
};
