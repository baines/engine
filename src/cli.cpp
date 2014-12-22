#include "cli.h"
#include "engine.h"

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

CLI::CLI(Engine& e)
: engine           (e)
, active           (false)
, ignore_next_text (false)
, show_cursor      (true)
, output_dirty     (false)
, blink_timer      (0)
, scrollback_lines (e.cfg.addVar("cli_scrollback_lines", CVarInt(64, 1, 8192)))
, visible_lines    (e.cfg.addVar("cli_visible_lines",    CVarInt(5, 1, 64)))
, font_height      (e.cfg.addVar("cli_font_height",      CVarInt(16, 8, 32)))
, cursor_blink_ms  (e.cfg.addVar("cli_cursor_blink_ms",  CVarInt(500, 100, 10000)))
, font             (e, { "DejaVuSansMono.ttf" }, font_height->val)
, output_text      ()
, output_lines     (scrollback_lines->val)
, output_line_idx  (0)
, input_text       (e, *font, { 0, font_height->val * visible_lines->val}, "> ")
, input_str        ("> ")
//FIXME: cursor position will break for non-monospace non-8x16 fonts.
, cursor_text      (e, *font, {font_height->val, 2 + font_height->val*visible_lines->val}, "_"){
	e.input.watchAction(this, "cli_submit",       ACT_SUBMIT);
	e.input.watchAction(this, "cli_backspace",    ACT_BACKSPACE);
	e.input.watchAction(this, "cli_autocomplete", ACT_AUTOCOMPLETE);
	e.input.watchAction(this, "cli_del_word",     ACT_DEL_WORD);
	e.input.watchAction(this, "console",          ACT_IGNORE_TEXT);

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

		echo(input_str);

		auto split_idx = input_str.find_first_of(' ', 2);
		if(split_idx == std::string::npos){
			split_idx = input_str.size();
		}

		const char* var_name = input_str.c_str() + 2;
		int var_name_sz = split_idx - 2;
		
		uint32_t hash = str_hash_len(var_name, var_name_sz);
		if(e.cfg.evalVar(hash, &input_str[split_idx])){
			echo("Ok.");
		} else {
			char buf[81] = {};
			size_t len = snprintf(buf, sizeof(buf)-1, "Unknown var \"%.*s\".", var_name_sz, var_name);
			echo(string_view(buf, len));
		}

		input_str.assign("> ");

	} else if(action == ACT_BACKSPACE && input_str.size() > 2){

		input_str.pop_back();

	} else if(action == ACT_DEL_WORD && input_str.size() > 2){
	
		size_t i = std::max<size_t>(2, input_str.find_last_not_of(' '));
		size_t j = std::max<size_t>(2, input_str.find_last_of(' ', i));

		input_str.erase(input_str.begin() + j, input_str.end());

	} else if(action == ACT_AUTOCOMPLETE){
		
		if(!e.cfg.extendPrefix(input_str, 2)){
			autocompletions.clear();
			e.cfg.getVarsWithPrefix(input_str.c_str()+2, autocompletions);

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

			} while(row_width > 80); //FIXME: don't hardcode 80 cols max.
			
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
	
	int i = output_line_idx;

	for(auto& t : output_text){
		if(output_dirty){
			if(--i < 0){
				i = output_lines.size() - 1;
			}
			t.update(output_lines[i]);
		}

		t.draw(r);
	}
	output_dirty = false;

	input_text.update(input_str);
	input_text.draw(r);

	if(show_cursor){
		updateCursor();
		cursor_text.draw(r);
	}
}

void CLI::echo(const string_view& str){
	output_lines[output_line_idx].assign(std::move(str.to_string()));
	output_line_idx = (output_line_idx + 1) % output_lines.size();
	output_dirty = true;
}

void CLI::updateCursor(){
	glm::ivec2 pos = cursor_text.getPos();
	glm::ivec2 newpos = glm::ivec2(input_str.size() * (font->getLineHeight()/2), pos.y);

	if(pos != newpos){
		cursor_text.update("_", newpos);
		show_cursor = true;
		blink_timer = 0;
	}
}

CLI::~CLI(){

}

