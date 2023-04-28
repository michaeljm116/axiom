#pragma once
#include <array>
#define MOUSE_BUTTON_DOWN 2
#define MOUSE_BUTTON_CHANGED 1

namespace axiom {
	static const uint32_t prev_action_bit = 2;
    struct Cmp_Mouse{
		double x;
		double y;
		double scroll;
		bool active;
		std::array<uint8_t,12> buttons;
	};
    struct Cmp_Keyboard{
		std::array<uint8_t,348> keys = {0};
	};
	
    struct Cmp_Controller{};


	//Bit 1 = the change bit
	//bit 2 = the mouse button state
	enum InputStates {
		NONE = 0x00,
		RELEASED = 0b01,
		HELD = 0b10,
		PRESSED = 0b11,
	};

	enum class Input_States{
		None = 0,
		Released = 1,
		Held = 2,
		Pressed = 3
	};

	/*
	void updateKey(Key key, bool pressed) {		
		bool prev_action = buttons[btn] & prev_action_bit;
		key.action.prev = key.action.curr & prev_action_bit;
		int change = prev_action != pressed;
		uint16_t change = key.action.prev != ac
		buttons[btn] = change + (pressed << 1);
		bool prev_action = key.action.curr & prev_action_bit;
		int change = prev_action != pressed;
		key.action
	};*/

}
