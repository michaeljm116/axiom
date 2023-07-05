#include "pch.h"
#include "sys-input.h"
#include "cmp-window.h"
namespace Axiom
{
	namespace Input{
		void initialize()
		{
			//set up kb & mouse
			g_world.set<Cmp_Keyboard>({});
			g_world.set<Cmp_Mouse>({});
			auto* mouse = g_world.get_mut<Cmp_Mouse>();
			auto* window = g_world.get<Cmp_Window>()->window;
			glfwGetCursorPos(window, &mouse->x, &mouse->y);

			//set up callbacks
			glfwSetKeyCallback(window, key_callback);
			glfwSetCharCallback(window, char_callback);
			glfwSetCursorPosCallback(window, cursor_position_callback);
			glfwSetInputMode(window, GLFW_STICKY_KEYS, true);
			glfwSetScrollCallback(window, scroll_callback);
			glfwSetJoystickCallback(joystick_callback);

			// Update Mouse
			g_world.system<Cmp_Mouse>("MouseSystem")
			.kind(flecs::OnUpdate)
			.each([](flecs::entity e, Cmp_Mouse& m){
				auto* window = g_world.get<Cmp_Window>()->window;
				int left_mb = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
				int right_mb = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
				update_button(m,GLFW_MOUSE_BUTTON_LEFT, left_mb);
				update_button(m,GLFW_MOUSE_BUTTON_RIGHT, right_mb);
			});
		}

		void update_button(Cmp_Mouse &mouse, int btn, bool pressed)
		{
			bool prev_action = mouse.buttons[btn] & prev_action_bit;
			int change = prev_action != pressed;
			mouse.buttons[btn] = change + (pressed << 1);
		}
	}
}