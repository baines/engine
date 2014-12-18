#include "cli.h"
#include "engine.h"

enum {
	ACT_SUBMIT,
	ACT_BACKSPACE,
	ACT_CURSOR_LEFT,
	ACT_CURSOR_RIGHT,
	ACT_CURSOR_UP,
	ACT_CURSOR_DOWN,
	ACT_IGNORE_TEXT,
	ACT_AUTOCOMPLETE
};

CLI::CLI(Engine& e)
: engine           (e)
, active           (false)
, ignore_next_text (false)
, scrollback_lines (e.cfg.addVar("cli_scrollback_lines", CVarInt(64, 1, 8192)))
, visible_lines    (e.cfg.addVar("cli_visible_lines",    CVarInt(5, 1, 64)))
, font_height      (e.cfg.addVar("cli_font_height",      CVarInt(16, 8, 32)))
, font             (e, { "DejaVuSansMono.ttf" }, font_height->val)
, output_text      ()
, output_buffer    (new char[80 * scrollback_lines->val])
, input_text       (e, *font, { 0, font_height->val * visible_lines->val}, "> ")
, input_str        ("> ") {
	e.input.watchAction(this, "cli_submit", ACT_SUBMIT);
	e.input.watchAction(this, "cli_backspace", ACT_BACKSPACE);
	e.input.watchAction(this, "cli_autocomplete", ACT_AUTOCOMPLETE);
	e.input.watchAction(this, "console", ACT_IGNORE_TEXT);
	e.input.enableText(this, true, { 0, font_height->val * (visible_lines->val + 1) });

	for(int i = 0; i < visible_lines->val; ++i){
		output_text.emplace_back(e, *font, glm::ivec2(0, font_height->val * i), ""); 
	}
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

	if(action == ACT_SUBMIT && !input_str.empty()){
		auto i = input_str.find_first_of(' ');
		if(i == std::string::npos){
			i = input_str.size();
		}
		uint32_t hash = str_hash_len(input_str.c_str(), i);
		e.cfg.evalVar(hash, &input_str[i]);
	} else if(action == ACT_BACKSPACE){
		if(input_str.size() > 2){
			input_str.pop_back();	
			input_text.update(input_str.c_str());
		}
	} else if(action == ACT_AUTOCOMPLETE){
		//TODO: do stuff with e.cfg.prefixExtend
	} else if(action == ACT_IGNORE_TEXT){
		ignore_next_text = true;
		return false; // let the event pass down to root_state which will close the console.
	}

	return true;
}

void CLI::onText(Engine& e, const char* text){
	if(ignore_next_text){
		ignore_next_text = false;
		return;
	}

	input_str += text;
	input_text.update(input_str.c_str());
}

void CLI::update(Engine& e, uint32_t delta){

}

void CLI::draw(Renderer& r){
	if(!active) return;
	for(auto& t : output_text){
		t.draw(r);		
	}
	input_text.draw(r);
}

CLI::~CLI(){

}

