#include "engine.h"
#include "root_state.h"
#include "input.h"
#include "cli.h"

enum {
	ACTION_QUIT,
	ACTION_TOGGLE_CONSOLE
};

RootState::RootState(Engine& e) {
	e.input->subscribe(this, "menu", ACTION_QUIT);
	e.input->subscribe(this, "console", ACTION_TOGGLE_CONSOLE);
}

void RootState::update(Engine& e, uint32_t delta){

}

void RootState::draw(Renderer& r){

}

bool RootState::onInput(Engine& e, int action_id, bool pressed){
	bool handled = false;
	
	if(action_id == ACTION_QUIT){
		e.quit();
		handled = true;
	} else if(pressed && action_id == ACTION_TOGGLE_CONSOLE){
		e.cli->toggle();
		handled = true;
	}
	
	return handled;
}

