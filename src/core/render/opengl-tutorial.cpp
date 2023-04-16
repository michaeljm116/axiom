#include "pch.h"
#include "opengl-tutorial.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
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
		layout(std140, binding = 0) uniform PerFrameData{
			uniform mat4 mvp;
			uniform int is_wireframe;
		};
		layout (location=0) out vec3 color;

		const vec3 pos[8] = vec3[8](
			vec3(-1.0,-1.0, 1.0), vec3( 1.0,-1.0, 1.0),
			vec3(1.0, 1.0, 1.0), vec3(-1.0, 1.0, 1.0),
			vec3(-1.0,-1.0,-1.0), vec3(1.0,-1.0,-1.0),
			vec3( 1.0, 1.0,-1.0), vec3(-1.0, 1.0,-1.0)
		);

		const vec3 col[8] = vec3[8](
			vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0),
			vec3(0.0, 0.0, 1.0), vec3(1.0, 1.0, 0.0),
			vec3(1.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0),
			vec3(0.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0)
		);

		const int indices[36] = int[36](
			 0, 1, 2, 2, 3, 0,
			 1, 5, 6, 6, 2, 1,
			 7, 6, 5, 5, 4, 7,
			 4, 0, 3, 3, 7, 4,
			 4, 5, 1, 1, 0, 4,
			 3, 2, 6, 6, 7, 3
		);


		void main(){
			int idx = indices[gl_VertexID];
			gl_Position = mvp * vec4(pos[idx], 1.0);
			color = is_wireframe > 0 ? vec3(0.0) : col[idx];
		};

	)";

	static const char* shaderCodeFragment = R"(
		#version 460 core
		layout (location=0) in vec3 color;
		layout (location=0) out vec4 out_FragColor;
		void main()
		{
			out_FragColor = vec4(color, 1.0);
		};
	)";

    //Link the shader ngayon
	const GLuint vrt_sdr = glCreateShader(GL_VERTEX_SHADER);
	const GLuint frg_sdr = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vrt_sdr, 1, &shaderCodeVertex, nullptr);
	glCompileShader(vrt_sdr);
	glShaderSource(frg_sdr, 1, &shaderCodeFragment, nullptr);
	glCompileShader(frg_sdr);

	//Verify shader comp
	GLint vsuccess;
	GLint fsuccess;
	glGetShaderiv(vrt_sdr, GL_COMPILE_STATUS, &vsuccess);
	glGetShaderiv(frg_sdr, GL_COMPILE_STATUS, &fsuccess);
	if (!vsuccess)
	{
		GLint logSize = 0;
		glGetShaderiv(vrt_sdr, GL_INFO_LOG_LENGTH, &logSize);
		std::vector<char> errorLog(logSize);
		glGetShaderInfoLog(vrt_sdr, logSize, &logSize, &errorLog[0]);
		std::string error(errorLog.begin(), errorLog.end());
		axiom::LogError(error);
	}
	if (!fsuccess)
	{
		GLint logSize = 0;
		glGetShaderiv(frg_sdr, GL_INFO_LOG_LENGTH, &logSize);
		std::vector<char> errorLog(logSize);
		glGetShaderInfoLog(frg_sdr, logSize, &logSize, &errorLog[0]);
		std::string error(errorLog.begin(), errorLog.end());
		axiom::LogError(error);
	}


    //Start the program
	program_ = glCreateProgram();
	glAttachShader(program_, vrt_sdr);
	glAttachShader(program_, frg_sdr);
	glLinkProgram(program_);
	glUseProgram(program_);

	
	//Create uniform buffer
	glCreateBuffers(1, &per_frame_data_buf);
	glNamedBufferStorage(per_frame_data_buf, k_buffer_size, nullptr, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, per_frame_data_buf, 0, k_buffer_size);
}

void OpenGLTutorial::Render() {
    // ... (Rendering code from OpenGLTutorialRender)
	glfwGetFramebufferSize(window_, &width_, &height_);
	glViewport(0,0,width_,height_);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    
	Update();
	glfwSwapBuffers(window_);

}

void OpenGLTutorial::Update(){

	//Calculate the matrices
	ratio_ = width_/ (float)height_;
	m_ = glm::rotate( glm::translate(glm::mat4(1.f), glm::vec3(0, 0, -3.5f)), (float)glfwGetTime(), glm::vec3(1.f, 1.f, 1.f));
	p_ = glm::perspective(45.f, ratio_, .1f, 1000.f);

	//mo ish
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.f, -1.f);

	per_frame_data = {.mvp_ = p_ * m_, .is_wireframe = false};
	glNamedBufferSubData(per_frame_data_buf, 0, k_buffer_size, &per_frame_data);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	per_frame_data.is_wireframe = true;
	glNamedBufferSubData(per_frame_data_buf, 0, k_buffer_size, &per_frame_data);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}

} // namespace axiom
