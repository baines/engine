#ifndef INPUT_H_
#define INPUT_H_
#include "common.h"
#include <SDL_rect.h>

extern struct mouse_button_tag_t {} mouse_button_tag;
extern struct mouse_wheel_tag_t {} mouse_wheel_tag;
extern struct pad_button_tag_t {} pad_button_tag;
extern struct mouse_tag_t {} mouse_tag;

struct SDL_Keysym;
struct SDL_ControllerDeviceEvent; 

struct InputKey {
	InputKey() = default;

	InputKey(uint32_t scancode);
	InputKey(const SDL_Keysym& key);

	InputKey(mouse_button_tag_t, uint8_t button); //TODO: multiple mice.
	InputKey(mouse_wheel_tag_t, int dir);

	InputKey(pad_button_tag_t, uint8_t button);

	InputKey(const char* str, bool raw_scancode = false);

	size_t toString(char* buf, size_t len);

	bool operator<(const InputKey& k) const;
	bool operator==(const InputKey& k) const;

	uint32_t code;
	bool shift, ctrl, alt, ignore_mods;
};

struct InputAxis {
	InputAxis() = default;

	//Axis(SDL_GameControllerAxis pad_axis);

	InputAxis(mouse_tag_t, int axis);

	InputAxis(const char* str);

	size_t toString(char* buf, size_t len);

	bool operator<(const InputAxis& a) const;
	bool operator==(const InputAxis& a) const;

	enum AxisType {
		AXIS_INVALID,
		AXIS_MOUSE,
		AXIS_PAD
	} type;
	int device_index;
	int axis_index;
};

struct IInput {
	
	virtual void bind(const InputKey& key, const StrRef& action) = 0;
	virtual void bind(const InputAxis& axis, const StrRef& action, bool rel, float scale = 1.0f)= 0;

	virtual void subscribe(GameState* s, const str_const& action, int action_id)= 0;
	//virtual void subscribe(GameState* s, const str_const& action, int action_id, const InputKey& dflt)= 0;
	virtual void enableText(GameState* s, bool enable, const SDL_Rect& pos = {0, 0, 0, 0})= 0;

	virtual void onDeviceChange(SDL_ControllerDeviceEvent& event) = 0;
	virtual void onStateChange(GameState* new_state) = 0;

	virtual bool getKeyAction(GameState* s, const InputKey& key, int& act_id) = 0;
	//bool getPadAction(GameState* s, SDL_JoystickID id, int button, int& action_id);
	virtual bool getAxisAction(GameState* s, const InputAxis& a, int& act_id, bool& rel, float& scale) = 0;

	virtual ~IInput(){}
};

#endif
