#ifndef CLI_H_
#define CLI_H_
#include "common.h"
#include "game_state.h"
#include "resource.h"
#include "font.h"
#include "text.h"
#include <vector>

struct CLI : public GameState {
	CLI(Engine& e);
	
	virtual bool onInput(Engine& e, int action, bool pressed);
	virtual void onText(Engine& e, const char* text);
	virtual void update(Engine& e, uint32_t delta);
	virtual void draw(Renderer& r);
	
	void toggle(void);
	bool execute(const char* line);
	void printf(const char* fmt, ...);
	
	~CLI();
private:
	Engine& engine;
	bool active;

	CVarInt* scrollback_lines;
	CVarInt* visible_lines;
	CVarInt* font_height;

	Resource<Font, uint16_t> font;
	
	std::vector<Text> output_text;
	std::unique_ptr<char[]> output_buffer;
	
	Text input_text;
	char input_buffer[256];
	char* input_cursor;
};

#endif

