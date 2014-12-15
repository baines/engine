#include "cli.h"
#include "engine.h"

CLI::CLI(Engine& e)
: engine(e)
, active(false)
, font(e, { "DejaVuSansMono.ttf" }, 16)
, lines() {
	char spaces[81] = {};

	for(int i = 0; i < 5; ++i){
		memset(spaces, 'a'+i, sizeof(spaces)-1);
		glm::ivec2 pos = { 0, 16 * i };
		lines.emplace_back(e, *font, pos, spaces);
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
	
	
	return false;
}

void CLI::update(Engine& e, uint32_t delta){
	puts("cli is open");
}

void CLI::draw(Renderer& r){
	if(!active) return;
	for(auto& l : lines){
		l.draw(r);		
	}
}

CLI::~CLI(){

}

