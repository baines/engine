#include "cli.h"
#include "engine.h"

/* TODO:
 * Cursor left / right:
 *     backspace -> delete from left of cursor exclusive
 *     delete    -> delete from right of cursor inclusive
 * Scroll through history:
 *     pageup    -> scroll 1 line up in history
 *     pagedown  -> scroll 1 line down in history
 *     Add some indication that it's not at the bottom.
 *         maybe replace bottom line with "v v v v v v v" or something.
 *         if/when Text supports colours, make it red.
 */

enum {
	ACT_SUBMIT,
	ACT_BACKSPACE,
	ACT_DEL_WORD,
	ACT_CURSOR_LEFT,
	ACT_CURSOR_RIGHT,
	ACT_CURSOR_UP,
	ACT_CURSOR_DOWN,
	ACT_IGNORE_TEXT,
	ACT_AUTOCOMPLETE
};

//TODO: dynamically adjust max cols based on screen size / cvar?
static const size_t MAX_COLS = 80;

CLI::CLI(Engine& e)
: engine           (e)
, active           (false)
, ignore_next_text (false)
, show_cursor      (true)
, output_dirty     (false)
, input_dirty      (false)
, blink_timer      (0)
, scrollback_lines (e.cfg.addVar<CVarInt>("cli_scrollback_lines", 64, 1, 8192))
, visible_lines    (e.cfg.addVar<CVarInt>("cli_visible_lines",    5, 1, 64))
, font_height      (e.cfg.addVar<CVarInt>("cli_font_height",      16, 8, 32))
, cursor_blink_ms  (e.cfg.addVar<CVarInt>("cli_cursor_blink_ms",  500, 100, 10000))
, font             (e, { "DejaVuSansMono.ttf" }, font_height->val)
, output_text      (e, *font, { 0, 0 }, "")
, output_lines     (scrollback_lines->val)
, output_line_idx  (0)
, input_text       (e, *font, { 0, font_height->val * visible_lines->val}, "> ")
, input_history    ()
, history_idx      (0)
, input_str        ("> ")
, cursor_text      (e, *font, input_text.getEndPos() + glm::ivec2(0, 2), "_"){
	e.input.watchAction(this, "cli_submit",       ACT_SUBMIT);
	e.input.watchAction(this, "cli_backspace",    ACT_BACKSPACE);
	e.input.watchAction(this, "cli_autocomplete", ACT_AUTOCOMPLETE);
	e.input.watchAction(this, "cli_del_word",     ACT_DEL_WORD);
	e.input.watchAction(this, "cli_cursor_up",    ACT_CURSOR_UP);
	e.input.watchAction(this, "cli_cursor_down",  ACT_CURSOR_DOWN);
	e.input.watchAction(this, "console",          ACT_IGNORE_TEXT);

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

	if(action == ACT_SUBMIT && input_str.size() > 2){

		echo(input_str);

		auto split_idx = input_str.find_first_of(' ', 2);
		if(split_idx == std::string::npos){
			split_idx = input_str.size();
		}

		const char* var_name = input_str.c_str() + 2;
		int var_name_sz = split_idx - 2;
		
		uint32_t hash = str_hash_len(var_name, var_name_sz);
		auto* v = e.cfg.getVar<CVar>(hash);

		if(v
		&& v->type != CVAR_FUNC 
		&& input_str.find_first_not_of(' ', split_idx) == std::string::npos){
			printVarInfo(*v);
		} else if(v && v->eval(&input_str[split_idx+1])){
			if(auto* str = v->getReloadVar()){
				echo({ "New value will take effect on ", str, "." });
			} else {
				// unsure if this is useful or annoying.
				// echo("Ok.");
			}
		} else if(v){
			echo(v->getErrorString());
		} else {
			char buf[MAX_COLS] = {};
			size_t len = snprintf(buf, sizeof(buf), "Unknown var \"%.*s\".", var_name_sz, var_name);
			echo(string_view(buf, len));
		}

		input_history.push_back(std::move(input_str));
		history_idx = input_history.size();
		input_str.assign("> ");
		input_dirty = true;

	} else if(action == ACT_BACKSPACE && input_str.size() > 2){

		while((input_str.back() & 0xC0) == 0x80){
			input_str.pop_back();
		}
		input_str.pop_back();
		input_dirty = true;

	} else if(action == ACT_DEL_WORD && input_str.size() > 2){
	
		size_t i = std::max<size_t>(2, input_str.find_last_not_of(' '));
		size_t j = std::max<size_t>(2, input_str.find_last_of(' ', i));

		input_str.erase(input_str.begin() + j, input_str.end());
		input_dirty = true;

	} else if(action == ACT_AUTOCOMPLETE && input_str.size() > 2){

		if(input_str.back() == ' '){
			uint32_t hash = str_hash_len(input_str.c_str()+2, input_str.size()-3);
			if(CVar* cvar = e.cfg.getVar<CVar>(hash)){
				printVarInfo(*cvar);
			}
			return true;
		}

		//TODO: maybe implement autocompletion of function args + enums too?
		
		autocompletions.clear();
		e.cfg.getVarsWithPrefix(input_str.c_str()+2, autocompletions);
		
		if(e.cfg.extendPrefix(input_str, 2)){
			if(autocompletions.size() == 1){
				input_str.append(1, ' ');
			}
			input_dirty = true;
		} else {

			// figure out a pretty column layout for the list of completions.
			size_t rows = 0, row_width;
			std::vector<size_t> col_widths; //FIXME: avoid allocating this.

			do {
				row_width = 1;
				col_widths.clear();

				for(size_t i = 0; i < autocompletions.size(); i += (rows+1)){
					col_widths.push_back(std::accumulate(
						autocompletions.begin() + i,
						autocompletions.begin() + i + rows + 1,
						0,
						[](size_t sz, CVar* c){
							return std::max(sz, c->name.size + 1);
						}
					));
					row_width += col_widths.back();
				}

				++rows;

			} while(row_width > MAX_COLS);
			
			echo(input_str);

			std::vector<std::string> lines(rows, " "); //FIXME: avoid allocating this.

			for(size_t i = 0; i < autocompletions.size(); ++i){
				const str_const& str = autocompletions[i]->name;

				lines[i % rows].append(str.str, str.size);
				
				if(i != autocompletions.size()-1){
					size_t num_spaces = col_widths[i / rows] - str.size;
					lines[i % rows].append(1, ',').append(num_spaces, ' ');
				}
			}

			for(auto&& l : lines){
				echo(l);
			}
		}

	} else if(action == ACT_CURSOR_UP){
	
		if(history_idx > 0){
			input_str = input_history[--history_idx];
			input_dirty = true;
		}
		
	} else if(action == ACT_CURSOR_DOWN){

		if(history_idx < input_history.size()){
			if(++history_idx == input_history.size()){
				input_str.assign("> ");
			} else {
				input_str = input_history[history_idx];
			}
			input_dirty = true;
		}

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
	input_dirty = true;
}

void CLI::update(Engine& e, uint32_t delta){
	blink_timer += delta;
	if(blink_timer > cursor_blink_ms->val){
		blink_timer = 0;
		show_cursor = !show_cursor;
	}

	if((size_t)font_height->val != font->getLineHeight()){
		font = Resource<Font, uint16_t>(e, { "DejaVuSansMono.ttf" }, font_height->val);
		output_dirty = true;
		input_dirty = true;
	}

}

void CLI::draw(Renderer& r){
	if(!active) return;
	
	if(output_dirty){
		std::string output_concat;
		for(int i = visible_lines->val; i > 0; --i){
			int idx = (output_line_idx + (output_lines.size() - i)) % output_lines.size();
			output_concat.append(output_lines[idx]).append(1, '\n');
		}
		output_text.update(std::move(output_concat));
		output_dirty = false;
	}
	output_text.draw(r);

	if(input_dirty){
		input_text.update(input_str, { 0, font_height->val * visible_lines->val });
		input_dirty = false;
	}
	input_text.draw(r);

	if(show_cursor){
		updateCursor();
		cursor_text.draw(r);
	}
}

void CLI::echo(const string_view& str){
	echo({ str });
}

void CLI::echo(std::initializer_list<string_view> strs){
	output_lines[output_line_idx].clear();
	
	for(auto& str : strs){
		output_lines[output_line_idx].append(str.data(), str.size());
	}
	output_line_idx = (output_line_idx + 1) % output_lines.size();
	output_dirty = true;
}

void CLI::printVarInfo(const CVar& cvar){
	char buf[MAX_COLS] = {};
	size_t off = snprintf(buf, sizeof(buf), " '%s' = ", cvar.name.str);

	cvar.printInfo(*this, buf, buf+off, sizeof(buf) - off);
}

void CLI::updateCursor(){
	glm::ivec2 pos = cursor_text.getStartPos();
	glm::ivec2 newpos = input_text.getEndPos() + glm::ivec2(0, 2);

	if(pos != newpos){
		cursor_text.update("_", newpos);
		show_cursor = true;
		blink_timer = 0;
	}
}

CLI::~CLI(){

}

