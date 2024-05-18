/*  button_box.ino
 *
 * Custom sketch for controlling a button box with Teensy 2.0++
 * 
 * @author  Jokke
 * @date    12-05-2024
 */

#define PRESS_DELAY 200
#define REPEATER_COUNT 4
#define BTN_COUNT (sizeof(buttons) / sizeof(buttons[0]))

#define BTN_ACTUAL_STATE(btn_ptr, btn_state) 
			(btn_ptr->inverted ? !btn_state : btn_state)

#define IS_TOGGLE_BTN(btn_num) 
			(btn_num == 2 || btn_num == 3 ||	\
			btn_num == 8 || btn_num == 9 || 	\
			btn_num == 14 || btn_num == 15)

enum ButtonType
{
	TOGGLE,
	CLICK,
	REPEATER
};

struct ButtonBoxItem
{
	ButtonType type;
	uint8_t pin;
	bool state;
	bool inverted;
	uint8_t num;
};

ButtonBoxItem buttons[23] = {
	{.type = ButtonType::CLICK, .pin = PIN_E1, .state = 0, .inverted = false, .num = 1},
	{.type = ButtonType::TOGGLE, .pin = PIN_C5, .state = 0, .inverted = true, .num = 2},
	{.type = ButtonType::TOGGLE, .pin = PIN_C6, .state = 0, .inverted = true, .num = 3},
	{.type = ButtonType::CLICK, .pin = PIN_B1, .state = 0, .inverted = true, .num = 4},
	{.type = ButtonType::CLICK, .pin = PIN_B2, .state = 0, .inverted = true, .num = 5},
	{.type = ButtonType::CLICK, .pin = PIN_D7, .state = 0, .inverted = false, .num = 6},
	{.type = ButtonType::CLICK, .pin = PIN_E0, .state = 0, .inverted = false, .num = 7},
	{.type = ButtonType::TOGGLE, .pin = PIN_C3, .state = 0, .inverted = true, .num = 8},
	{.type = ButtonType::TOGGLE, .pin = PIN_C4, .state = 0, .inverted = true, .num = 9},
	{.type = ButtonType::CLICK, .pin = PIN_E7, .state = 0, .inverted = true, .num = 10},
	{.type = ButtonType::CLICK, .pin = PIN_B0, .state = 0, .inverted = true, .num = 11},
	{.type = ButtonType::CLICK, .pin = PIN_D6, .state = 0, .inverted = false, .num = 12},
	{.type = ButtonType::CLICK, .pin = PIN_C0, .state = 0, .inverted = false, .num = 13},
	{.type = ButtonType::TOGGLE, .pin = PIN_C1, .state = 0, .inverted = true, .num = 14},
	{.type = ButtonType::TOGGLE, .pin = PIN_C2, .state = 0, .inverted = true, .num = 15},
	{.type = ButtonType::CLICK, .pin = PIN_C7, .state = 0, .inverted = true, .num = 16},
	{.type = ButtonType::CLICK, .pin = PIN_E6, .state = 0, .inverted = true, .num = 17},
	{.type = ButtonType::CLICK, .pin = PIN_D0, .state = 0, .inverted = true, .num = 18},
	{.type = ButtonType::CLICK, .pin = PIN_D1, .state = 0, .inverted = true, .num = 19},
	{.type = ButtonType::CLICK, .pin = PIN_D2, .state = 0, .inverted = false, .num = 20},
	{.type = ButtonType::CLICK, .pin = PIN_D3, .state = 0, .inverted = false, .num = 21},
	{.type = ButtonType::CLICK, .pin = PIN_D4, .state = 0, .inverted = false, .num = 22},
	{.type = ButtonType::CLICK, .pin = PIN_D5, .state = 0, .inverted = false, .num = 23}
};

int current_profile = 1;

void setup()
{
	Serial.begin(9600);

	for (uint8_t i = 0; i < BTN_COUNT; ++i) {
		ButtonBoxItem* btn = &buttons[i];
		pinMode(btn->pin, INPUT_PULLUP);

		btn->state = BTN_ACTUAL_STATE(btn, digitalRead(btn->pin));
	}
}

void btn_click(ButtonBoxItem* btn)
{
	uint8_t curr_btn_state = BTN_ACTUAL_STATE(btn, digitalRead(btn->pin));
	if (btn->state == curr_btn_state) return;

	btn->state = curr_btn_state;

	Joystick.button(btn->num, true);
	delay(PRESS_DELAY);
	Joystick.button(btn->num, false);
}

void btn_repeater(ButtonBoxItem* btn)
{
	uint8_t curr_btn_state = BTN_ACTUAL_STATE(btn, digitalRead(btn->pin));
	if (btn->state == curr_btn_state) return;

	btn->state = curr_btn_state;

	for (uint8_t i = 0; i < REPEATER_COUNT; ++i) {
		Joystick.button(btn->num, true);
		delay(PRESS_DELAY);
		Joystick.button(btn->num, false);
		delay(PRESS_DELAY);
	}
}

void btn_toggle(ButtonBoxItem* btn)
{
	btn->state = BTN_ACTUAL_STATE(btn, digitalRead(btn->pin));
	Joystick.button(btn->num, btn->state);
}

void load_default_profile()
{
	for (uint8_t i = 0; i < BTN_COUNT; ++i) {
		ButtonBoxItem* btn = &buttons[i];

		if (IS_TOGGLE_BTN(btn->num)) {
			btn->type = ButtonType::TOGGLE;
		} else {
			btn->type = ButtonType::CLICK;
			Joystick.button(btn->num, false);
		}
	}

	current_profile = 1;
	Serial.println("Default profile loaded!");
}

void load_toggle_profile()
{
	for (uint8_t i = 0; i < BTN_COUNT; ++i) {
		ButtonBoxItem* btn = &buttons[i];
		btn->type = ButtonType::TOGGLE;
	}

	current_profile = 2;
	Serial.println("Toggle profile loaded!");
}

void load_repeater_profile()
{
	for (uint8_t i = 0; i < BTN_COUNT; ++i) {
		ButtonBoxItem* btn = &buttons[i];

		if (IS_TOGGLE_BTN(btn->num)) {
			btn->type = ButtonType::TOGGLE;
		} else {
			btn->type = ButtonType::REPEATER;
		}
	}

	current_profile = 3;
	Serial.println("Repeater profile loaded!");
}

bool handle_profile_switch(ButtonBoxItem* btn1, ButtonBoxItem* btn2)
{
	uint8_t state1 = BTN_ACTUAL_STATE(btn1, digitalRead(btn1->pin));
	if (!state1) return false;

	uint8_t state2 = BTN_ACTUAL_STATE(btn2, digitalRead(btn2->pin));
	if (!state2) return false;

	// TOP buttons
	if (btn1->num == 2 && btn2->num == 3 && current_profile != 1) {
		load_default_profile();
		return true;
	}

	// MIDDLE buttons
	if (btn1->num == 8 && btn2->num == 9 && current_profile != 2) {
		load_toggle_profile();
		return true;
	}

	// BOTTOM buttons
	if (btn1->num == 14 && btn2->num == 15 && current_profile != 3) {
		load_repeater_profile();
		return true;
	}

	return false;
}

void loop()
{
	// Handle TOP, MIDDLE, BOTTOM button combos.
	if (handle_profile_switch(&buttons[1], &buttons[2])) return;
	if (handle_profile_switch(&buttons[7], &buttons[8])) return;
	if (handle_profile_switch(&buttons[13], &buttons[14])) return;

	for (uint8_t i = 0; i < BTN_COUNT; ++i) {
		ButtonBoxItem* btn = &buttons[i];

		switch (btn->type) {
			case CLICK: btn_click(btn); break;
			case REPEATER: btn_repeater(btn); break;
			case TOGGLE: btn_toggle(btn); break;
			default: break;
		}
	}
}
