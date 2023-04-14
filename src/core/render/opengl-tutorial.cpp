#include "opengl-tutorial.h"
#include <iostream>

namespace axiom {

OpenGLTutorial::OpenGLTutorial(GLFWwindow* window) : window_(window) {}

OpenGLTutorial::~OpenGLTutorial() {
    glDeleteProgram(program_);
    glDeleteVertexArrays(1, &vao_);
}

void OpenGLTutorial::Init() {
    // ... (Initialization code from OpenGLTutorialInit)
	glCreateVertexArrays(1, &vao_);
	glBindVertexArray(vao_);

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

    //Start the program
	program_ = glCreateProgram();
	glAttachShader(program_, vrt_sdr);
	glAttachShader(program_, frg_sdr);
	glLinkProgram(program_);
	glUseProgram(program_);
}

void OpenGLTutorial::Render() {
    // ... (Rendering code from OpenGLTutorialRender)
    int width, height;
	glfwGetFramebufferSize(window_, &width, &height);
	glViewport(0,0,width,height);
	glClearColor(1.f, 1.f, 1.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glfwSwapBuffers(window_);
}

} // namespace axiom
