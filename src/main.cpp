#include <iostream>
#include <flecs.h>
#include <glm/glm.hpp>
#include "helpers.h"

int main(){
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
		std::cout << "Position X:" << e.get<Position>()->x << " Y: " << e.get<Position>()->y;
		if (i > 1000)
			break;
	}

	return 0;
}

