#include "cli.h"
#include "engine.h"

enum {
	ACT_SUBMIT,
	ACT_BACKSPACE,
	ACT_AUTOCOMPLETE
};

CLI::CLI(Engine& e)
: engine           (e)
, active           (false)
, scrollback_lines (e.cfg.addVar("cli_scrollback_lines", CVarInt(64, 1, 8192)))
, visible_lines    (e.cfg.addVar("cli_visible_lines",    CVarInt(5, 1, 64)))
, font_height      (e.cfg.addVar("cli_font_height",      CVarInt(16, 8, 32)))
, font             (e, { "DejaVuSansMono.ttf" }, font_height->val)
, output_text      ()
, output_buffer    (new char[80 * scrollback_lines->val])
, input_text       ()
, input_buffer     ()
, input_cursor     (input_buffer) {

	e.input.watchAction(this, "cli_submit", ACT_SUBMIT);
	e.input.watchAction(this, "cli_backspace", ACT_BACKSPACE);
	e.input.watchAction(this, "cli_autocomplete", ACT_AUTOCOMPLETE);

	e.input.enableText(this, true, { 0, font_height->val * (visible_lines->val + 1) });
}

void CLI::toggle(){
	if(active){
		engine.state.pop(1);
		//XXX: what if it's not top-most?
	} else {
		engine.state.push(this);
	}
	
	active = !active;
}

bool CLI::onInput(Engine& e, int action, bool pressed){
	if(!pressed) return true;

	if(action == ACT_SUBMIT){
		char* ptr = strchr(input_buffer, ' ');
		if(!ptr) ptr = input_cursor;

		uint32_t hash = str_hash_len(input_buffer, ptr - input_buffer);
		e.cfg.evalVar(hash, ptr);

	} else if(action == ACT_BACKSPACE){
		input_cursor = std::max<char*>(input_buffer, input_cursor - 1);
		*input_cursor = '\0';

		input_text.update(input_buffer);

	} else if(action == ACT_AUTOCOMPLETE){
		//TODO: do stuff with e.cfg.prefixExtend
	}

	return true;
}

void CLI::onText(Engine& e, const char* text){
	// 
}

void CLI::update(Engine& e, uint32_t delta){

}

void CLI::draw(Renderer& r){
	if(!active) return;
	for(auto& t : output_text){
		t.draw(r);		
	}
}

CLI::~CLI(){

}

