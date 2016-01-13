#ifndef CLI_PRIVATE_H_
#define CLI_PRIVATE_H_
#include "cli.h"
#include "shader.h"
#include "material.h"
#include "sprite.h"
#include "sprite_batch.h"
#include "text.h"
#include <vector>

struct CLI : public ICLI {
	CLI(Engine& e);
	
	bool onInput(Engine& e, int action, bool pressed);
	void onText(Engine& e, const char* text);
	void onStateChange(Engine& e, bool activated);
	void onResize(Engine& e, int w, int h);
	void update(Engine& e, uint32_t delta);
	void draw(IRenderer& r);
	
	void toggle(void);
	bool execute(const char* line);
	
	void echo(const StrRef& str);
	void echo(const std::initializer_list<StrRef> str);
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
	std::vector<StrMut> output_lines;
	size_t output_line_idx;
	size_t scroll_offset;

	Text input_text;
	std::vector<StrMut> input_history; //XXX unbounded history is probably a bad idea..
	size_t history_idx;
	StrMut input_str;

	Text cursor_text;
	size_t cursor_idx;

	std::vector<CVar*> autocompletions;
};

#endif

