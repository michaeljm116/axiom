#include "pch.h"
#include "sys-input.h"
#include "cmp-window.h"
namespace axiom
{

	Sys_Input::Sys_Input(){
		Init();
		g_world.system<Cmp_Mouse>("MouseSystem")
		.kind(flecs::OnUpdate)
		.each([this](flecs::entity e, Cmp_Mouse& m){
			this->Update(e, m);
		});

	}
    Sys_Input::~Sys_Input()
    {
    }

	void Init()
	{
		//set up kb & mouse
		g_world.set<Cmp_Keyboard>({});
		g_world.set<Cmp_Mouse>({});
		auto* mouse = g_world.get_mut<Cmp_Mouse>();
		auto* window = g_world.get<Cmp_Window>()->window;
		glfwGetCursorPos(window, &mouse->x, &mouse->y);

		//set up callbacks
		glfwSetKeyCallback(window, Sys_Input::key_callback);
		glfwSetCharCallback(window, Sys_Input::char_callback);
		glfwSetCursorPosCallback(window, Sys_Input::cursor_position_callback);
		glfwSetInputMode(window, GLFW_STICKY_KEYS, true);
		glfwSetScrollCallback(window, Sys_Input::scroll_callback);
		glfwSetJoystickCallback(Sys_Input::joystick_callback);

	}

	void Sys_Input::Update(flecs::entity e, Cmp_Mouse& mouse)
	{
		auto* window = g_world.get<Cmp_Window>()->window;
		int left_mb = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
		int right_mb = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
		UpdateButton(mouse,GLFW_MOUSE_BUTTON_LEFT, left_mb);
		UpdateButton(mouse,GLFW_MOUSE_BUTTON_RIGHT, right_mb);
	}
    void Sys_Input::UpdateButton(Cmp_Mouse &mouse, int btn, bool pressed)
    {
		bool prev_action = mouse.buttons[btn] & prev_action_bit;
		int change = prev_action != pressed;
		mouse.buttons[btn] = change + (pressed << 1);
    }
}