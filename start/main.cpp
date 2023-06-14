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
#include "sys-vulkan-boilerplate.h"

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
#include "render-base.h"


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

	axiom::log::Init();
	axiom::timer::Init();
	axiom::resource::Init();
	axiom::window::Init("Axiom Engine", 1280, 720);


	g_world.add<axiom::render::Cmp_Vulkan>();
	auto vc = g_world.get_ref<axiom::render::Cmp_Vulkan>();
	axiom::render::base::InitializeVulkan(*vc.get());

	g_world.add<axiom::Cmp_CurrentTime>();
	g_world.add<axiom::Cmp_LogFile>();
	g_world.add<axiom::Cmp_Timer>();
	


	vulkany::Init();
	vulkany::Check(r, "Initializing Volk");



	std::string assets_folder = "../../assets/";
	auto t = g_world.entity("Models Timer");
	auto at = g_world.entity("Animations TImer");


	//Load Models
	t.add<axiom::Cmp_Timer>();
	axiom::resource::LoadDirectory(assets_folder + "Models");
	t.remove<axiom::Cmp_Timer>();

	//Load Animations
	at.add<axiom::Cmp_Timer>();
	axiom::resource::LoadDirectory(assets_folder + "Animations");
	at.remove<axiom::Cmp_Timer>();

	//Load Materials
	axiom::resource::LoadMaterials(assets_folder + "Materials.xml");


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
		bp.set<axiom::resource::Mesh>({d});
		bp.child_of(e_froku);
		body_parts.push_back(bp);
	}

	for(auto bp : body_parts){
		std::cout << bp.name() << std::endl;
	}
	auto head = g_world.lookup("Froku::head");
	auto h_cmp = head.get<axiom::resource::Mesh>();
	auto afro = e_froku.lookup("afro");
	afro.child_of(head);
	auto a_cmp = afro.get<axiom::resource::Mesh>();

	auto prnt = afro.parent();
	auto prnt_cmp = prnt.get<axiom::Cmp_Res_Model>();

	//auto* twindow = g_world.get<Cmp_Window>()->window;

	axiom::transform::Init();

	auto sun_trans = Cmp_Transform(glm::vec3(1,1,1), glm::vec3(0), glm::vec3(1));
	auto earth_trans = Cmp_Transform(glm::vec3(5,5,5), glm::vec3(45), glm::vec3(.1));
	auto moon_trans = Cmp_Transform(glm::vec3(2,2,2), glm::vec3(33), glm::vec3(.5));
	auto sun = g_world.entity("Sun")
		.add<Cmp_Transform, Cmp_Static>()
		.set<Cmp_Transform, Cmp_Static>(sun_trans);

	auto earth = g_world.entity("Earth")
		.child_of(sun)
		.add<Cmp_Transform, Cmp_Static>()
		.set<Cmp_Transform, Cmp_Static>(earth_trans);

	auto moon = g_world.entity("Moon")
		.child_of(earth)
		.add<Cmp_Transform, Cmp_Static>()
		.set<Cmp_Transform, Cmp_Static>(moon_trans);

	g_world.each([](flecs::entity e, flecs::pair<Cmp_Transform, Cmp_Static> p){
				std::cout << e.name() << ": {" << p->local.pos.x << "}\n";
			});

	scene::init();
	g_world.set<axiom::Cmp_Scene>({"../../assets/Scenes/", "TestEntrance.xml", 0});

	g_world.progress();
	g_world.each([](flecs::entity e, Cmp_Transform p){
				//std::stringstream ssl;
				//ssl << e.name() << ": {" << p.local.pos.x << ", " << p.local.pos.y << ", " << p.local.pos.z << "}";
				//axiom::log::Set(axiom::log::Level::DEBUG, ssl.str());
				//e.add<Cmp_Static>();
			});
			
	axiom::log::Set(axiom::log::Level::DEBUG, "-------STATIC-------");
	g_world.each([](flecs::entity e, Cmp_Transform p, Cmp_Static s){
		std::stringstream ssl;
		ssl << e.name() << ": {" << p.local.pos.x << ", " << p.local.pos.y << ", " << p.local.pos.z << "}";
		axiom::log::Set(axiom::log::Level::DEBUG, ssl.str());
	});
	axiom::log::Set(axiom::log::Level::DEBUG, "-------DYNAMIC-------");
	g_world.each([](flecs::entity e, Cmp_Transform p,  Cmp_Dynamic d){
		std::stringstream ssl;
		ssl << e.name() << ": {" << p.local.pos.x << ", " << p.local.pos.y << ", " << p.local.pos.z << "}";
		axiom::log::Set(axiom::log::Level::DEBUG, ssl.str());
	});

	//while(!glfwWindowShouldClose(twindow)){
		g_world.progress();
		

		
	//}
	//axiom::window::Destruct();
};