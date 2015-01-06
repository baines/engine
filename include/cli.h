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
	virtual void onStateChange(Engine& e, bool activated);
	virtual void update(Engine& e, uint32_t delta);
	virtual void draw(Renderer& r);
	
	void toggle(void);
	bool execute(const char* line);
	
	void echo(const string_view& str);
	void echo(const std::initializer_list<string_view> str);
	void printVarInfo(const CVar& cvar);

	~CLI();
private:
	void updateCursor();

	Engine& engine;
	bool toggling, active, ignore_next_text, show_cursor, output_dirty, input_dirty;
	int blink_timer;

	CVarInt* scrollback_lines;
	CVarInt* visible_lines;
	CVarInt* font_height;
	CVarInt* cursor_blink_ms;

	Resource<Font, uint16_t> font;
	
	Text output_text;
	std::vector<std::string> output_lines;
	size_t output_line_idx;
	
	Text input_text;
	std::vector<std::string> input_history; //XXX unbounded history is probably a bad idea..
	size_t history_idx;
	std::string input_str;

	Text cursor_text;

	std::vector<CVar*> autocompletions;
};

#endif

