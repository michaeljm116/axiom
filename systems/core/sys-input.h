#pragma once
#include "cmp-input.h"
#include <flecs-world.h>
#include <GLFW/glfw3.h>
#include <cmath>

namespace Axiom
{
    namespace Input
    {
		void initialize();
		inline void update_button(Cmp_Mouse& mouse, int btn, bool pressed);

		static void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods)
		{
			//Get Keyboard
			auto* keyboard = g_world.get_mut<Cmp_Keyboard>();

			//Handle the changes
			bool prev_action = keyboard->keys[key] & prev_action_bit;
			bool pressed = action;
			int change = prev_action != pressed;
			keyboard->keys[key] = change + (pressed << 1);	
#ifdef UIIZON
			ImGuiIO& io = ImGui::GetIO();
			if (action == GLFW_PRESS || action == GLFW_REPEAT) {
				io.KeysDown[key] = true;
			}
			else if (action == GLFW_RELEASE) {
				io.KeysDown[key] = false;
			}
#endif // 
		}

		static void char_callback(GLFWwindow*, unsigned int c)
		{
#ifdef UIIZON
			ImGuiIO& io = ImGui::GetIO();
			io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
			io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
			io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
			if (c > 0 && c < 0x10000)
				io.AddInputCharacter((unsigned short)c);
#endif // UIIZON
		}

		static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
			auto* mouse = g_world.get_mut<Cmp_Mouse>();
			mouse->prev_x = mouse->x;
			mouse->prev_y = mouse->y;
			mouse->x = xpos;
			mouse->y = ypos;
		}
		
		static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
			auto* mouse = g_world.get_mut<Cmp_Mouse>();
			mouse->prev_scroll = mouse->scroll;
			mouse->scroll += yoffset;
		}
		static void joystick_callback(int jid, int event) {
			if (event == GLFW_CONNECTED)
			{
				// The joystick was connected
			}
			else if (event == GLFW_DISCONNECTED)
			{
				// The joystick was disconnected
			}
		}
    }
}
