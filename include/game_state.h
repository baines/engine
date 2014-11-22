#ifndef GAME_STATE_H_
#define GAME_STATE_H_
#include "common.h"

struct GameState {
	
	virtual bool onInit(Engine& e){ return true; }
	virtual int  onQuit(Engine& e){ return 0; }
	
	// state is now / no longer active (top-most)
	virtual void onStateChange(Engine& e, bool now_active){} 

	// window focus gained / lost
	virtual void onFocus(Engine& e, bool gained){}
	
	virtual bool onInput(Engine& e, int action, bool pressed){ 
		return false;
	}
	
	virtual bool onMotion(Engine& e, int axis_id, int rel_x, int rel_y){
		return false;
	}
	
	virtual void update(Engine& e, uint32_t delta) = 0;
	virtual void draw(Renderer& r) = 0;
	
	virtual ~GameState(){};
};

#endif
