#include <iostream>
#include <flecs.h>
#include <glm/glm.hpp>
#include "helpers.h"
#include "sys-movement.h"

int main(){

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
	


	glm::vec3 lalala(0, 1, 2);
	std::cout << "Hello, World!" << lalala.x  << lalala.z << std::endl;
	
	flecs::world ecs;
	ecs.system<Position, const Velocity>().each([](Position& p, const Velocity& v){
		p.x += v.x;
		p.y += v.y;
	});
	
	auto e = ecs.entity().set(([](Position& p, Velocity& v){
		p = {10, 20};
		v = {1, 2};
	}));
	int i = 0;
	while (ecs.progress()){
		++i;
		std::cout << "\nPosition X:" << e.get<Position>()->x << " Y: " << e.get<Position>()->y;
		if (i > 1000)
			break;
	}

	return 0;
}

