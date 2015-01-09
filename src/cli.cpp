#include "cli.h"
#include "engine.h"

/* TODO:
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
	ACT_DELETE,
	ACT_DEL_WORD,
	ACT_CURSOR_LEFT,
	ACT_CURSOR_RIGHT,
	ACT_CURSOR_UP,
	ACT_CURSOR_DOWN,
	ACT_HOME,
	ACT_END,
	ACT_AUTOCOMPLETE,
	ACT_IGNORE_TEXT
};

//TODO: dynamically adjust max cols based on screen size / cvar?
static const size_t MAX_COLS  = 80;
static const char   PROMPT[]  = "> ";
static const size_t PROMPT_SZ = sizeof(PROMPT) - 1;
static const char   CURSOR[]  = "_";

CLI::CLI(Engine& e)
: engine           (e)
, toggling         (false)
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
, input_text       (e, *font, { 0, font_height->val * visible_lines->val}, PROMPT)
, input_history    ()
, history_idx      (0)
, input_str        (PROMPT)
, cursor_text      (e, *font, input_text.getEndPos() + glm::ivec2(0, 2), CURSOR)
, cursor_idx       (PROMPT_SZ)
, autocompletions  () {
	e.input.watchAction(this, "cli_submit",       ACT_SUBMIT);
	e.input.watchAction(this, "cli_backspace",    ACT_BACKSPACE);
	e.input.watchAction(this, "cli_delete",       ACT_DELETE);
	e.input.watchAction(this, "cli_del_word",     ACT_DEL_WORD);
	e.input.watchAction(this, "cli_cursor_up",    ACT_CURSOR_UP);
	e.input.watchAction(this, "cli_cursor_down",  ACT_CURSOR_DOWN);
	e.input.watchAction(this, "cli_cursor_left",  ACT_CURSOR_LEFT);
	e.input.watchAction(this, "cli_cursor_right", ACT_CURSOR_RIGHT);
	e.input.watchAction(this, "cli_home",         ACT_HOME);
	e.input.watchAction(this, "cli_end",          ACT_END);
	e.input.watchAction(this, "cli_autocomplete", ACT_AUTOCOMPLETE);
	e.input.watchAction(this, "console",          ACT_IGNORE_TEXT);

	e.input.enableText(this, true, { 0, font_height->val * (visible_lines->val + 1) });
}

void CLI::onStateChange(Engine& e, bool activated){
	active = activated;
	toggling = false;
}

void CLI::toggle(){
	if(toggling) return;

	if(active){
		engine.state.pop(1);
		//XXX: what if it's not top-most?
	} else {
		engine.state.push(this);
	}

	toggling = true;
}

bool CLI::onInput(Engine& e, int action, bool pressed){
	if(!pressed) return true;

	if(action == ACT_SUBMIT && input_str.size() > PROMPT_SZ){

		echo(input_str);

		auto split_idx = input_str.find_first_of(' ', PROMPT_SZ);
		if(split_idx == std::string::npos){
			split_idx = input_str.size();
		}

		const char* var_name = input_str.c_str() + PROMPT_SZ;
		int var_name_sz = split_idx - PROMPT_SZ;
		
		uint32_t hash = str_hash_len(var_name, var_name_sz);
		auto* v = e.cfg.getVar<CVar>(hash);

		if(v
		&& v->type != CVAR_FUNC 
		&& input_str.find_first_not_of(' ', split_idx) == std::string::npos){
			printVarInfo(*v);
		} else if(v && v->eval(&input_str[std::min(input_str.size(), split_idx+1)])){
			if(auto* str = v->getReloadVar()){
				echo({ " New value will take effect on ", str, "." });
			} else {
				// unsure if this is useful or annoying.
				// echo("Ok.");
			}
		} else if(v){
			echo(v->getErrorString());
		} else {
			this->printf(" Unknown var \"%.*s\".\n", var_name_sz, var_name);
		}

		input_history.push_back(std::move(input_str));
		history_idx = input_history.size();
		input_str.assign(PROMPT);
		input_dirty = true;

	} else if(action == ACT_BACKSPACE && cursor_idx > PROMPT_SZ){

		size_t end_idx = utf8_char_index(input_str, cursor_idx);
		size_t start_idx = end_idx - 1;
		for(; start_idx >= PROMPT_SZ; --start_idx){
			if(!((input_str[start_idx] & 0xC0) == 0x80)) break;
		}

		input_str.erase(input_str.begin() + start_idx, input_str.begin() + end_idx);
		input_dirty = true;

	} else if(action == ACT_DELETE){
		
		size_t i = utf8_char_index(input_str, cursor_idx);
		if(i < input_str.size()){
			do {
				input_str.erase(i, 1);
			} while((input_str[i] & 0xC0) == 0x80);

			input_dirty = true;
			++cursor_idx;
		}	
		
	} else if(action == ACT_DEL_WORD && input_str.size() > PROMPT_SZ){

		size_t end_idx = utf8_char_index(input_str, cursor_idx);
		size_t mid_idx = std::max(PROMPT_SZ, input_str.find_last_not_of(' ', end_idx));
		size_t beg_idx = std::max(PROMPT_SZ, input_str.find_last_of(' ', mid_idx));

		input_str.erase(input_str.begin() + beg_idx, input_str.begin() + end_idx);
		input_dirty = true;

	} else if(action == ACT_AUTOCOMPLETE && input_str.size() > PROMPT_SZ){

		if(input_str.back() == ' '){
			uint32_t hash = str_hash_len(
				input_str.c_str() + PROMPT_SZ,
				input_str.size() - (PROMPT_SZ+1)
			);
			if(CVar* cvar = e.cfg.getVar<CVar>(hash)){
				printVarInfo(*cvar);
			}
			return true;
		}

		//TODO: maybe implement autocompletion of function args + enums too?
		
		autocompletions.clear();
		e.cfg.getVarsWithPrefix(input_str.c_str()+PROMPT_SZ, autocompletions);
		
		if(e.cfg.extendPrefix(input_str, PROMPT_SZ)){
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

				size_t ac_sz = autocompletions.size();
				for(size_t i = 0; i < ac_sz; i += (rows+1)){
					col_widths.push_back(std::accumulate(
						autocompletions.begin() + i,
						autocompletions.begin() + std::min(ac_sz, i + rows + 1),
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
				input_str.assign(PROMPT);
			} else {
				input_str = input_history[history_idx];
			}
			input_dirty = true;
		}

	} else if(action == ACT_CURSOR_LEFT){
	
		cursor_idx = std::max<int>(PROMPT_SZ, cursor_idx - 1);
		
	} else if(action == ACT_CURSOR_RIGHT){
	
		cursor_idx = std::min<int>(input_text.size(), cursor_idx + 1);

	} else if(action == ACT_HOME){
		
		cursor_idx = PROMPT_SZ;
		
	} else if(action == ACT_END){
	
		cursor_idx = input_text.size();

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
	
	size_t i = utf8_char_index(input_str, cursor_idx);
	input_str.insert(i, text);
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

	if((size_t)scrollback_lines->val != output_lines.size()){
		output_lines.resize(scrollback_lines->val);
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
		int char_diff = input_text.update(
			input_str, 
			{ 0, font_height->val * visible_lines->val }
		);
		cursor_idx = clamp<int>(cursor_idx + char_diff, PROMPT_SZ, input_text.size());
		input_dirty = false;
	}
	input_text.draw(r);

	updateCursor();
	if(show_cursor){
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

void CLI::printf(const char* fmt, ...){
	va_list v;
	va_start(v, fmt);
	char buf[MAX_COLS] = {};
	vsnprintf(buf, sizeof(buf), fmt, v);
	const char *start = buf, *end = "";

	do {
		end = strchrnul(start, '\n');
		output_lines[output_line_idx].append(start, end);
		if(*end){
			start = end + 1;
			output_line_idx = (output_line_idx + 1) % output_lines.size();
			output_lines[output_line_idx].clear();
		}
	} while(*end);

	output_dirty = true;
	va_end(v);
}

void CLI::printVarInfo(const CVar& cvar){
	this->printf(" '%s' = ", cvar.name.str);
	cvar.printInfo(*this);
}

void CLI::updateCursor(){
	glm::ivec2 pos = cursor_text.getStartPos();
	glm::ivec2 newpos = input_text.getPos(cursor_idx) + glm::ivec2(0, 2);

	if(pos != newpos){
		cursor_text.update(CURSOR, newpos);
		show_cursor = true;
		blink_timer = 0;
	}
}

CLI::~CLI(){

}

