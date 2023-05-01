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

	axiom::log::Init();
	axiom::timer::Init();
	axiom::resource::Init();
	axiom::window::Init("Axiom Engine", 1280, 720);

	g_world.add<axiom::Cmp_CurrentTime>();
	g_world.add<axiom::Cmp_LogFile>();
	g_world.add<axiom::Cmp_Timer>();
	
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




            auto static_transform = g_world.query_builder<const Cmp_Transform, Cmp_Transform>()
                .term_at(1).second<Cmp_Static>()
                .term_at(2).second<Cmp_Static>()
                .term_at(2).parent().cascade().optional()
                .build();
            
            /*auto dynamic_transform = g_world.query_builder<const Cmp_Transform, Cmp_Transform>()
                .term_at(1).second<Cmp_Dynamic>()
                .term_at(2).second<Cmp_Dynamic>()
                .term_at(2).parent().cascade().optional()
                .build();*/

            static_transform.iter([](flecs::iter& it, const Cmp_Transform *parent, Cmp_Transform* child)
            {
                //build rotaiton matrix
                glm::mat4 rot_m = glm::rotate(glm::radians(child->euler_rot.x), glm::vec3(1.f, 0.f, 0.f));
                rot_m = glm::rotate(rot_m, glm::radians(child->euler_rot.y), glm::vec3(0.f, 1.f, 0.f));
                rot_m = glm::rotate(rot_m, glm::radians(child->euler_rot.z), glm::vec3(0.f, 0.f, 1.f));
                child->local.rot = rot_m;
                child->global.rot *= child->local.rot;

                //build position and scale matrix.
                glm::mat4 pos_m = glm::translate(glm::vec3(child->local.pos));
                glm::mat4 sca_m = glm::scale(glm::vec3(child->local.sca));
                glm::mat4 local = pos_m * rot_m;

                //combine them into 1 and multiply by parent if u haz parent
                if(parent){
                    child->global.sca = child->local.sca * parent->global.sca;
                    child->trm = parent->world * local;
                    local *= sca_m;
                    child->world = parent->world * local;
                }
                else{ //There's no parent 
                    child->global.sca = child->local.sca;
                    child->trm = local;
                    local *= sca_m;
                    child->world = local;
                }
            });


	g_world.each([](flecs::entity e, flecs::pair<Cmp_Transform, Cmp_Static> p){
				std::cout << e.name() << ": {" << p->local.pos.x << "}\n";
			});

	while(!glfwWindowShouldClose(twindow)){
		g_world.progress();

		
	}
	axiom::window::Destruct();
};