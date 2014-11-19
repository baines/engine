#include "engine.h"

enum {
	ACTION_QUIT
};

RootState::RootState(Engine& e)
: engine(e) {
	e.input.watchAction(this, "menu", ACTION_QUIT);
}

void RootState::update(Engine& e, uint32_t delta){

}

void RootState::draw(Renderer& r){

}

bool RootState::onInput(int action_id, bool pressed){
	if(action_id == ACTION_QUIT){
		engine.quit();
		return true;
	} else {
		return false;
	}
}

