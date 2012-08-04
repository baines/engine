#ifndef _CONSOLE_H_
#define _CONSOLE_H_
#include "gamestate.h"
#include "statemgr.h"

class Console {
public:
	Console(StateMgr& s) : statemgr(s), 
		console_state(new ConsoleState(s, *this)), shown(false){}
	void show(){
		if(shown) return;
		console_state->show();
		statemgr.push(console_state);
		shown = true;
	}
	void hide(){
		if(!shown) return;
		console_state->hide();
	}
	void toggle(void){
		shown ? hide() : show();
	}
private:
	class ConsoleState : public Gamestate {
	public:
		ConsoleState(StateMgr& s, Console& c) : Gamestate(s), console(c), 
			bg("console.png", 160, -20, 320, 40), anim(0) {};
		void show(){ anim = 1; }
		void hide(){ anim = -1; }
		void update(Input& in, uint32_t delta){
			//TODO: text input.
			if(anim){
				bg.y += (int)delta * anim * 0.2f;
				if(anim > 0 && bg.y > 20.0f){
					bg.y = 20.0f;
					anim = 0;
				} else if(anim < 0 && bg.y < -20.0f){
					console.shown = false;
					statemgr.pop(1);
				}
			}
		}
		void draw(std::vector<uint32_t>& gfx){
			bg.draw(gfx);
		}
	private:
		Console& console;
		Sprite bg;
		int anim;
	};
	friend class ConsoleState;
	std::shared_ptr<ConsoleState> console_state;
	bool shown;
	StateMgr& statemgr;
};

#endif
