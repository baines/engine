#include "engine.h"

RootState::RootState(Engine& e)
: engine(e) {
	e.input.watchAction(this, "QUIT", 1);
}

void RootState::update(Engine& e, uint32_t delta){

}

void RootState::draw(Renderer& r){

}

bool RootState::onInput(int action_id, bool pressed){
	if(action_id == 1){
		engine.quit();
		return true;
	} else {
		return false;
	}
}

