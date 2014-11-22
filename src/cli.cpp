#include "cli.h"
#include "engine.h"

CLI::CLI(Engine& e)
: engine(e)
, active(false) {

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

}

CLI::~CLI(){

}

