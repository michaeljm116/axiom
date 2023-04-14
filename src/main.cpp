#include "pch.h"
#include <flecs.h>
#include <flecs/addons/flecs_cpp.h>
#include <glm/glm.hpp>
#include "helpers.h"
#include "sys-movement.h"
#include "core/render/window.h"
#include "core/util/log.h"


void FlecsTutorial();
void OpenGLTutorialInit();
void OpenGLTutorialRender();

int main(){

	axiom::SWindow().Init();
	OpenGLTutorialInit();
	while(!glfwWindowShouldClose(axiom::SWindow().GetWindow())){
		OpenGLTutorialRender();
		glfwPollEvents();
	}

	return 0;
	
}

void OpenGLTutorialInit(){
	//You need an array of verts
	GLuint VAO;
	glCreateVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//make a smol tri shader
	static const char* shaderCodeVertex = R"(
		#version 460 core
		layout (location=0) out vec3 color;

		const vec2 pos[3] = vec2[3](
			vec2(-0.6, -0.4),
			vec2(0.6, -0.4),
			vec2(0.0, 0.6)
		);
		const vec3 col[3] = vec3[3](
			vec3(1.0, 0.0, 0.0),
			vec3(0.0, 1.0, 0.0),
			vec3(0.0, 0.0, 1.0)
		);

		void main(){
			gl_Position = vec4(pos[gl_VertexID], 0.0, 1.0);
			color = col[gl_VertexID];
		}

	)";

	static const char* shaderCodeFragment = R"(
		#version 460 core
		layout (location=0) in vec3 color;
		layout (location=0) out vec4 out_FragColor;
		void main(){
			out_FragColor = vec4(color, 1.0);
		};
	)";

	//Link the shader ngayon
	const GLuint vrt_sdr = glCreateShader(GL_VERTEX_SHADER);
	const GLuint frg_sdr = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vrt_sdr, 1, &shaderCodeVertex, nullptr);
	glShaderSource(frg_sdr, 1, &shaderCodeFragment, nullptr);
	glCompileShader(vrt_sdr);
	glCompileShader(frg_sdr);

	const GLuint program = glCreateProgram();
	glAttachShader(program, vrt_sdr);
	glAttachShader(program, frg_sdr);
	glLinkProgram(program);
	glUseProgram(program);
};

void OpenGLTutorialRender(){
	int width, height;
	glfwGetFramebufferSize(axiom::SWindow().GetWindow(), &width, &height);
	glViewport(0,0,width,height);
	glClearColor(1.f, 1.f, 1.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glfwSwapBuffers(axiom::SWindow().GetWindow());
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

	
	//i haven't comprehended what this does yet but...
	//flecs::entity pos_e = world.id<Position>();	
	//std::cout << "Name: " << pos_e.name() << std::endl;


	//tbh still not sure what a component on this is liek what does this do?
	//const EcsComponent *c = me.get<flecs::Component>();
	//std::cout << "Component size: " << c->size << std::endl;
};
