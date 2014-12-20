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
, show_cursor      (true)
, blink_timer      (0)
, scrollback_lines (e.cfg.addVar("cli_scrollback_lines", CVarInt(64, 1, 8192)))
, visible_lines    (e.cfg.addVar("cli_visible_lines",    CVarInt(5, 1, 64)))
, font_height      (e.cfg.addVar("cli_font_height",      CVarInt(16, 8, 32)))
, cursor_blink_ms  (e.cfg.addVar("cli_cursor_blink_ms",  CVarInt(500, 100, 10000)))
, font             (e, { "DejaVuSansMono.ttf" }, font_height->val)
, output_text      ()
, output_buffer    (new char[80 * scrollback_lines->val])
, input_text       (e, *font, { 0, font_height->val * visible_lines->val}, "> ")
, input_str        ("> ")
//FIXME: cursor position will break for non-monospace non-8x16 fonts.
, cursor_text      (e, *font, {font_height->val, 2 + font_height->val*visible_lines->val}, "_"){
	e.input.watchAction(this, "cli_submit", ACT_SUBMIT);
	e.input.watchAction(this, "cli_backspace", ACT_BACKSPACE);
	e.input.watchAction(this, "cli_autocomplete", ACT_AUTOCOMPLETE);
	e.input.watchAction(this, "console", ACT_IGNORE_TEXT);
	e.input.enableText(this, true, { 0, font_height->val * (visible_lines->val + 1) });

	for(int i = visible_lines->val-1; i >= 0; --i){
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

	if(action == ACT_SUBMIT && input_str.size() > 2){

		for(int i = output_text.size()-1; i >= 2; --i){
			output_text[i].update(output_text[i-2].getStr());
		}
		output_text[1].update(input_str);

		auto split_idx = input_str.find_first_of(' ', 2);
		if(split_idx == std::string::npos){
			split_idx = input_str.size();
		}

		const char* var_name = input_str.c_str() + 2;
		int var_name_sz = split_idx - 2;
		
		uint32_t hash = str_hash_len(var_name, var_name_sz);
		if(e.cfg.evalVar(hash, &input_str[split_idx])){
			output_text[0].update("Ok.");	
		} else {
			char buf[81] = {};

			snprintf(buf, sizeof(buf)-1, "Unknown var \"%.*s\".", var_name_sz, var_name);
			output_text[0].update(buf);
		}

		input_str.assign("> ");
		input_text.update(input_str);

		updateCursor();

	} else if(action == ACT_BACKSPACE && input_str.size() > 2){

		input_str.pop_back();
		input_text.update(input_str);

		updateCursor();

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
	updateCursor();
}

void CLI::update(Engine& e, uint32_t delta){
	blink_timer += delta;
	if(blink_timer > cursor_blink_ms->val){
		blink_timer = 0;
		show_cursor = !show_cursor;
	}
}

void CLI::draw(Renderer& r){
	if(!active) return;
	
	for(auto& t : output_text){
		t.draw(r);		
	}
	input_text.draw(r);

	if(show_cursor){
		cursor_text.draw(r);
	}
}

void CLI::updateCursor(){
	glm::ivec2 pos = cursor_text.getPos();
	cursor_text.update("_", glm::ivec2(input_str.size() * (font->getLineHeight()/2), pos.y));

	show_cursor = true;
	blink_timer = 0;
}

CLI::~CLI(){

}

