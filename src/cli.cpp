#include "cli_private.h"
#include "engine.h"
#include "config.h"
#include "font.h"
#include "state_system.h"
#include "input.h"
#include <algorithm>

enum {
	ACT_SUBMIT,
	ACT_BACKSPACE,
	ACT_DELETE,
	ACT_DEL_WORD,
	ACT_SCROLL_UP,
	ACT_SCROLL_DOWN,
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
, bg_scroll_timer  (0)
, scrollback_lines (e.cfg->addVar<CVarInt>("cli_scrollback_lines", 64, 1, 8192))
, visible_lines    (e.cfg->addVar<CVarInt>("cli_visible_lines",    8, 1, 64))
, font_height      (e.cfg->addVar<CVarInt>("cli_font_height",      16, 8, 32))
, cursor_blink_ms  (e.cfg->addVar<CVarInt>("cli_cursor_blink_ms",  500, 100, 10000))
, prev_vis_lines   (visible_lines->val)
, font             (e, { "DejaVuSansMono.ttf" }, font_height->val)
, bg_vs            (e, { "cli_bg.glslv" })
, bg_fs            (e, { "cli_bg.glslf" })
, bg_shader        (bg_vs, bg_fs)
, bg_material      (bg_shader)
, bg_batch         (bg_material)
, bg_sprite        (bg_batch)
, output_text      (e, font, { 0, 0 }, "")
, output_lines     (scrollback_lines->val)
, output_line_idx  (0)
, scroll_offset    (0)
, input_text       (e, font, { 0, font_height->val * visible_lines->val}, PROMPT)
, input_history    ()
, history_idx      (0)
, input_str        (PROMPT)
, cursor_text      (e, font, input_text.getEndPos() + vec2i{0, 2}, CURSOR)
, cursor_idx       (PROMPT_SZ)
, autocompletions  () {
	e.input->subscribe(this, "cli_submit",       ACT_SUBMIT);
	e.input->subscribe(this, "cli_backspace",    ACT_BACKSPACE);
	e.input->subscribe(this, "cli_delete",       ACT_DELETE);
	e.input->subscribe(this, "cli_del_word",     ACT_DEL_WORD);
	e.input->subscribe(this, "cli_scroll_up",    ACT_SCROLL_UP);
	e.input->subscribe(this, "cli_scroll_down",  ACT_SCROLL_DOWN);
	e.input->subscribe(this, "cli_cursor_up",    ACT_CURSOR_UP);
	e.input->subscribe(this, "cli_cursor_down",  ACT_CURSOR_DOWN);
	e.input->subscribe(this, "cli_cursor_left",  ACT_CURSOR_LEFT);
	e.input->subscribe(this, "cli_cursor_right", ACT_CURSOR_RIGHT);
	e.input->subscribe(this, "cli_home",         ACT_HOME);
	e.input->subscribe(this, "cli_end",          ACT_END);
	e.input->subscribe(this, "cli_autocomplete", ACT_AUTOCOMPLETE);
	e.input->subscribe(this, "console",          ACT_IGNORE_TEXT);

	logging::addSink([](logging::level l, const char* msg, size_t len, void* usr){
		CLI* cli = static_cast<CLI*>(usr);

		if(l == logging::warn || l == logging::error){
			cli->echo({ TXT_YELLOW, lvl_str(l), StrRef(msg, len), TXT_WHITE });
		}
	}, this);

	int w = e.cfg->getVar<CVarInt>("vid_width")->val,
	   	h = font_height->val * (visible_lines->val + 1);

	e.input->enableText(this, true, SDL_Rect{ 0, h });

	bg_shader.link();
	bg_sprite.setSize({ w, 4 + h });
	bg_sprite.setPosition({ w / 2, 2 + h / 2 });
	bg_material.uniforms.setUniform("height", { h + 4.0f });
}

void CLI::onStateChange(Engine& e, bool activated){
	active = activated;
	toggling = false;

	if(active){
		// this can probably be handled better somehow...
		onResize(e, e.cfg->getVar<CVarInt>("vid_width")->val, 0);
		ignore_next_text = false;
	}
}

void CLI::onResize(Engine& e, int w, int h){
	bg_sprite.setSize({ w, bg_sprite.getSize().y });
	bg_sprite.setPosition({ w / 2, bg_sprite.getPosition().y });
}

void CLI::toggle(){
	if(toggling) return;

	if(active){
		engine.state->pop(1);
		//XXX: what if it's not top-most?
	} else {
		engine.state->push(this);
	}

	toggling = true;
}

bool CLI::onInput(Engine& e, int action, bool pressed){
	if(!pressed) return false;

	if(action == ACT_SUBMIT && input_str.size() > PROMPT_SZ){

		echo(input_str);

		char* var_args = std::find(input_str.begin() + PROMPT_SZ, input_str.end(), ' ');
		const char* var_name = input_str.c_str() + PROMPT_SZ;

		int var_name_sz = (var_args - input_str.begin()) - PROMPT_SZ;
		if(var_args != input_str.end()) ++var_args;
		
		int var_idx = var_args - input_str.begin();

		uint32_t hash = str_hash_len(var_name, var_name_sz);
		auto* v = e.cfg->getVar<CVar>(hash);

		if(v && v->type != CVAR_FUNC && !input_str.contains_not(' ', var_idx)){
			printVarInfo(*v);
		} else if(v && v->eval(var_args)){
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
		input_str = PROMPT;
		input_dirty = true;

	} else if(action == ACT_BACKSPACE && cursor_idx > PROMPT_SZ){

		size_t end_idx = utf8_char_index(input_str, cursor_idx);
		size_t start_idx = end_idx - 1;
		for(; start_idx >= PROMPT_SZ; --start_idx){
			if(!is_utf8_continuation(input_str[start_idx])) break;
		}

		input_str.erase(input_str.begin() + start_idx, input_str.begin() + end_idx);
		input_dirty = true;

	} else if(action == ACT_DELETE){
		
		size_t i = utf8_char_index(input_str, cursor_idx);
		if(i < input_str.size()){
			do {
				input_str.erase(i, 1);
			} while(is_utf8_continuation(input_str[i]));

			input_dirty = true;
			++cursor_idx;
		}	
		
	} else if(action == ACT_DEL_WORD && input_str.size() > PROMPT_SZ){

		size_t end = utf8_char_index(input_str, cursor_idx);
		size_t mid = input_str.rfind_not(' ', end, PROMPT_SZ);
		size_t beg = input_str.rfind_any(' ', mid, PROMPT_SZ);

		input_str.erase(beg, end - beg);
		input_dirty = true;

	} else if(action == ACT_AUTOCOMPLETE && input_str.size() > PROMPT_SZ){

		if(input_str.back() == ' '){
			uint32_t hash = str_hash_len(
				input_str.c_str() + PROMPT_SZ,
				input_str.size() - (PROMPT_SZ+1)
			);
			if(CVar* cvar = e.cfg->getVar<CVar>(hash)){
				printVarInfo(*cvar);
			}
			return true;
		}

		//TODO: maybe implement autocompletion of function args + enums too?
		
		autocompletions.clear();
		e.cfg->getVarsWithPrefix(input_str.c_str()+PROMPT_SZ, autocompletions);
		
		if(e.cfg->extendPrefix(input_str, PROMPT_SZ)){
			if(autocompletions.size() == 1){
				input_str.append(1, ' ');
			}
			input_dirty = true;
		} else {

			// figure out a pretty column layout for the list of completions.
			size_t rows = 0, row_width;
			std::vector<int> col_widths;

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

			size_t cols = col_widths.size();

			for(size_t i = 0; i < rows; ++i){
				for(size_t j = 0; i < cols; ++j){
					size_t index = j * rows + i;
					if(index >= autocompletions.size()) break;
					const str_const& str = autocompletions[index]->name;

					printf(" %*.*s", -col_widths[j], (int)str.size, str.str);

				}
				printf("\n");
			}
		}

	} else if(action == ACT_SCROLL_UP){
		
		scroll_offset = std::min<int>(
			scroll_offset + 1,
			output_lines.size() - visible_lines->val
		);
		output_dirty = true;

	} else if(action == ACT_SCROLL_DOWN){
		
		scroll_offset = std::max<int>(0, scroll_offset - 1);
		output_dirty = true;

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

	bool font_size_changed = (size_t) font_height->val != font->getLineHeight();

	if(font_size_changed){
		font = { e, { "DejaVuSansMono.ttf" }, (size_t)font_height->val };
		output_dirty = true;
		input_dirty = true;
	}

	if((size_t)scrollback_lines->val != output_lines.size()){
		output_lines.resize(scrollback_lines->val);
	}

	if(font_size_changed || (size_t)visible_lines->val != prev_vis_lines){
		int height = font_height->val * (visible_lines->val + 1);

		bg_sprite.setSize({ bg_sprite.getSize().x, 4 + height });
		bg_sprite.setPosition({ bg_sprite.getPosition().x, 2 + height / 2 });
		bg_material.uniforms.setUniform("height", { height + 4.0f });

		prev_vis_lines = visible_lines->val;
	}

	bg_scroll_timer += delta;
	bg_material.uniforms.setUniform("timer",  { bg_scroll_timer / 40.0f });
}

void CLI::draw(IRenderer& r){
	if(!active) return;
	
	bg_batch.draw(r);
	
	// draw the input line + cursor if not scrolled up.
	if(input_dirty){
		int char_diff = input_text.update(
			input_str, 
			{ 0, font_height->val * visible_lines->val }
		);
		cursor_idx = clamp<int>(cursor_idx + char_diff, PROMPT_SZ, input_text.size());
		input_dirty = false;

		// if the input changed and we're scrolled up, force scroll to bottom.
		if(scroll_offset != 0){
			scroll_offset = 0;
			output_dirty = true;
		}
	}
	if(scroll_offset == 0) input_text.draw(r);

	updateCursor();
	if(show_cursor && scroll_offset == 0){
		cursor_text.draw(r);
	}
	if(output_dirty){
		StrMut output_concat;
		for(int i = visible_lines->val; i > 0; --i){
			int idx = (output_line_idx + (output_lines.size() - scroll_offset - i)) % output_lines.size();
			output_concat.append(output_lines[idx]).append(1, '\n');
		}

		// replace the input line with arrows if we're scrolled up.
		if(scroll_offset){
			output_concat.append(TXT_RED);
			for(size_t i = 0; i < MAX_COLS; ++i){
				output_concat.append("v ");
			}
			output_concat.append(TXT_WHITE);
		}

		output_text.update(output_concat);
		output_dirty = false;
	}
	output_text.draw(r);
}

void CLI::echo(const StrRef& str){
	echo({ str });
}

void CLI::echo(std::initializer_list<StrRef> strs){
	for(auto& str : strs){
		StrRef::const_iterator begin = str.begin(), end;
		do {
			end = std::find(begin, str.end(), '\n');
			output_lines[output_line_idx].append(StrRef(begin, end));
			begin = end + 1;
			if(end != str.end()){
				output_line_idx = (output_line_idx + 1) % output_lines.size();
				output_lines[output_line_idx].clear();
			}
		} while(end != str.end());
	}
	output_line_idx = (output_line_idx + 1) % output_lines.size();
	output_lines[output_line_idx].clear();
	output_dirty = true;
}

void CLI::printf(const char* fmt, ...){
	va_list v;
	va_start(v, fmt);
	char buf[MAX_COLS] = {};
	vsnprintf(buf, sizeof(buf), fmt, v);
	const char *start = buf, *end;

	do {
		end = strchrnul(start, '\n');
		output_lines[output_line_idx].append(StrRef(start, end));
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
	vec2i pos = cursor_text.getStartPos();
	vec2i newpos = input_text.getPos(cursor_idx) + vec2i{0, 2};

	if(pos != newpos){
		cursor_text.update(CURSOR, newpos);
		show_cursor = true;
		blink_timer = 0;
	}
}

CLI::~CLI(){

}

