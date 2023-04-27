#pragma once
#include <array>
#define MOUSE_BUTTON_DOWN 2
#define MOUSE_BUTTON_CHANGED 1

namespace axiom {
    struct Cmp_Mouse{
		int x;
		int y;
		double scroll;
		bool active;
		std::array<Button,12> buttons;
	};
    struct Cmp_Keyboard{
		std::array<Key,348> keys;
	};
    struct Cmp_Controller{};

	struct  Key{
		int action = 0;
		int cycles = 0; //cycle = cycle * action + action; fma(cycle,action,action)
	};

	struct Button{
		int action = 0;
		int cycles = 0;
	};
	//Bit 1 = the change bit
	//bit 2 = the mouse button state
	enum MouseStates {
		MOUSE_NONE = 0x00,
		MOUSE_RELEASED = 0b01,
		MOUSE_HELD = 0b10,
		MOUSE_PRESSED = 0b11,
	};

	/*
	* So lets list the changes
	* First youre idle
	* then you click and you press
	* as you remain you held
	* then you let go and you release
	* youve been released and now youre idle
	* 
	* how to determine a change has occured
	* if prev state != curr state
	* bit 1 = the comparison of the prev and current
	* bit 2 = the current
	* 
	* int prev_action_bit = 2
	* int change_bit = 1
	* bool prev_action = state & prev_action_bit
	* bool change = prev_action != curr_action;
	* state = change + (curr_action << 1);
	*/

	struct Mouse {
		int x;
		int y;
		int prevX;
		int prevY;

		int buttons[12];
		int buttons_prev[12];

		double scroll;
		bool   active;

		static const int prev_action_bit = 2;

		Mouse() {
			x = 0;
			y = 0;
			prevX = 0;
			prevY = 0;

			scroll = 0;
			active = true;

			for (int i = 0; i < 12; ++i) {
				buttons[i] = 0;
			}
		}
		 
		void updateButton(int btn, bool pressed) {
			if (active) {
				bool prev_action = buttons[btn] & prev_action_bit;
				int change = prev_action != pressed;
				buttons[btn] = change + (pressed << 1);
				/*if (pressed) {
					if (buttons[btn] & MOUSE_NONE)
						buttons[btn] = MOUSE_HELD | MOUSE_PRESSED;
					else if (buttons[btn] & MOUSE_PRESSED)
						buttons[btn] = MOUSE_HELD;
				}
				else { //not pressed
					if (buttons[btn] & MOUSE_PRESSED | MOUSE_HELD)
						buttons[btn] = MOUSE_RELEASED | MOUSE_NONE;
					else if (buttons[btn] & MOUSE_RELEASED | MOUSE_NONE)
						buttons[btn] = MOUSE_NONE;
				}*/
			}
		};

		void updatePosition(int _x, int _y) {
			if (active) {
				prevX = x;
				prevY = y;
				x = _x;
				y = _y;
			}
		}

		void updateScroll(double offset) {
			if (active)
				scroll = offset;
		}

	};
}

namespace Principia {
	enum KeyStates {
		KEY_NONE = 0x01,
		KEY_PRESSED = 0x02,
		KEY_HELD = 0x04,
		KEY_RELEASED = 0x08,
	};



	struct Key {
		unsigned char state;
		int key;
		std::string name;

		Key() { state = KEY_NONE; };
		Key(int key, std::string name) : key(key), name(name) {};
		void update(bool pressed) {
			if (pressed) {
				if (state & KEY_NONE)
					state = KEY_HELD | KEY_PRESSED;
				else if (state & KEY_PRESSED)
					state = KEY_HELD;
			}
			else
			{
				if (state & KEY_PRESSED | KEY_HELD)
					state = KEY_RELEASED | KEY_NONE;
				else if (state & KEY_RELEASED | KEY_NONE)
					state = KEY_NONE;
			}
		};
	};
}