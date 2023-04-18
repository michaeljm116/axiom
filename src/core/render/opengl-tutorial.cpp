#include "pch.h"
#include "opengl-tutorial.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

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

    //Link the shader ngayon
	const GLuint vrt_sdr = glCreateShader(GL_VERTEX_SHADER);
	const GLuint frg_sdr = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vrt_sdr, 1, &sdr_code_vert_textured, nullptr);
	glCompileShader(vrt_sdr);
	glShaderSource(frg_sdr, 1, &sdr_code_frag_textured, nullptr);
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

	LoadTexture();
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

void OpenGLTutorial::ScreenShot(){
	int w, h;
	glfwGetFramebufferSize(window_, &w, &h);
	uint8_t* ptr = (uint8_t*)malloc(w * h * 4);
	glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, ptr);
	auto t = axiom::Timer();
	auto f = std::string("../screenshot.png");
	axiom::Check(stbi_write_png(f.c_str(), w, h, 4, ptr, 0), "screenshotting " + f);
	free(ptr);
}

void OpenGLTutorial::LoadTexture()
{
	std::string path = "../../assets/RoadTexture.png";
	tut.img = stbi_load(path.c_str(), &tut.w, &tut.h, &tut.comp, 3);
	axiom::Check(tut.img != nullptr, path);
	glCreateTextures(GL_TEXTURE_2D, 1, &tut.texture);
	glTextureParameteri(tut.texture, GL_TEXTURE_MAX_LEVEL, 0);
	glTextureParameteri(tut.texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(tut.texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureStorage2D(tut.texture, 1, GL_RGB8, tut.w, tut.h);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTextureSubImage2D(tut.texture, 0, 0, 0, tut.w, tut.h, GL_RGB, GL_UNSIGNED_BYTE, tut.img);
	glBindTextures(0,1,&tut.texture);
}

void OpenGLTutorial::InitImGUI()
{
	/*
	// Create Buffers
	glCreateVertexArrays(1, &vao_);
	glCreateBuffers(1, &handle_vbo);
	glNamedBufferStorage(handle_vbo, 256 * 1024, nullptr, GL_DYNAMIC_STORAGE_BIT);
	glCreateBuffers(1, &handle_elements);
	glNamedBufferStorage(handle_elements, 256 * 1024, nullptr, GL_DYNAMIC_STORAGE_BIT);

	// Set up the vertex array attributes
	glVertexArrayElementBuffer(vao_, handle_elements);
	glVertexArrayVertexBuffer(vao_, 0, handle_vbo, 0, sizeof(ImDrawVert));
	glEnableVertexArrayAttrib(vao_, 0);
	glEnableVertexArrayAttrib(vao_, 1);
	glEnableVertexArrayAttrib(vao_, 2);

	glVertexArrayAttribFormat(vao_, 0, 2, GL_FLOAT, GL_FALSE, 		 IM_OFFSETOF(ImDrawVert, pos));
	glVertexArrayAttribFormat(vao_, 1, 2, GL_FLOAT, GL_FALSE, 		 IM_OFFSETOF(ImDrawVert, uv));
	glVertexArrayAttribFormat(vao_, 2, 4, GL_UNSIGNED_BYTE, GL_TRUE, IM_OFFSETOF(ImDrawVert, col));

	glVertexArrayAttribBinding(vao_, 0, 0);
	glVertexArrayAttribBinding(vao_, 1, 0);
	glVertexArrayAttribBinding(vao_, 2, 0);
	glBindVertexArray(vao_);

	// bind the shaders
	const GLuint vrt_sdr = glCreateShader(GL_VERTEX_SHADER);
	const GLuint frg_sdr = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vrt_sdr, 1, &imgui_sdr_vert, nullptr);
	glCompileShader(vrt_sdr);
	glShaderSource(frg_sdr, 1, &imgui_sdr_frag, nullptr);
	glCompileShader(frg_sdr);

    //Start the program
	program_ = glCreateProgram();
	glAttachShader(program_, vrt_sdr);
	glAttachShader(program_, frg_sdr);
	glLinkProgram(program_);
	glUseProgram(program_);


	//Begin the IMGUI Stuff now
	ImGui::CreateContext();
	auto& io = ImGui::GetIO();
	io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
	ImFontConfig cfg = ImFontConfig();
	cfg.FontDataOwnedByAtlas = false;
	cfg.RasterizerMultiply = 1.5f;
	cfg.SizePixels = 768.f/32.f;
	cfg.PixelSnapH = true;
	cfg.OversampleH = 4;
	cfg.OversampleV = 4;
	ImFont* font = io.Fonts->AddFontFromFileTTF("data/OpenSans-Light.ttf", cfg.SizePixels, &cfg);
	*/


}

} // namespace axiom
