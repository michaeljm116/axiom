#include "pch.h"
#include <flecs.h>
#include <flecs/addons/flecs_cpp.h>
#include <glm/glm.hpp>
#include "helpers.h"
#include "sys-movement.h"
#include "core/render/window.h"
#include "core/util/log.h"
#include "opengl-tutorial.h"
#include <optick.h>


void FlecsTutorial();

int main(){

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

	return 0;
	
}


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
