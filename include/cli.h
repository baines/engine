#ifndef CLI_H_
#define CLI_H_
#include "common.h"
#include "game_state.h"
#include "resource.h"
#include "font.h"
#include "text.h"
#include <vector>
#include "material.h"
#include "sprite_batch.h"
#include "sprite.h"
#include "shader.h"

struct CLI : public GameState {
	CLI(Engine& e);
	
	virtual bool onInput(Engine& e, int action, bool pressed);
	virtual void onText(Engine& e, const char* text);
	virtual void onStateChange(Engine& e, bool activated);
	virtual void onResize(Engine& e, int w, int h);
	virtual void update(Engine& e, uint32_t delta);
	virtual void draw(Renderer& r);
	
	void toggle(void);
	bool execute(const char* line);
	
	void echo(const string_view& str);
	void echo(const std::initializer_list<string_view> str);
	void printf(const char* fmt, ...) __attribute__ ((format (printf, 2, 3)));
	void printVarInfo(const CVar& cvar);

	~CLI();
private:
	void updateCursor();

	Engine& engine;
	bool toggling, active, ignore_next_text, show_cursor, output_dirty, input_dirty;
	int blink_timer, bg_scroll_timer;

	CVarInt* scrollback_lines;
	CVarInt* visible_lines;
	CVarInt* font_height;
	CVarInt* cursor_blink_ms;

	size_t prev_vis_lines;

	Resource<Font, size_t> font;
	Resource<VertShader> bg_vs;
	Resource<FragShader> bg_fs;
	ShaderProgram bg_shader;
	Material bg_material;
	SpriteBatch bg_batch;
	Sprite bg_sprite;
	
	Text output_text;
	std::vector<std::string> output_lines;
	size_t output_line_idx;
	size_t scroll_offset;

	Text input_text;
	std::vector<std::string> input_history; //XXX unbounded history is probably a bad idea..
	size_t history_idx;
	std::string input_str;

	Text cursor_text;
	size_t cursor_idx;

	std::vector<CVar*> autocompletions;
};

#endif

