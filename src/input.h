#ifndef _INPUT_H_
#define _INPUT_H_

#include <SDL/SDL_keyboard.h>
#include <SDL/SDL_mouse.h>

#define MOUSE_LEFT SDL_BUTTON(SDL_BUTTON_LEFT)
#define MOUSE_RIGHT SDL_BUTTON(SDL_BUTTON_RIGHT)

struct Mouse {
	int x;
	int y;
	int buttons;
};

class Input {
public:
	Uint8* keys;
	Uint8 mods;
	struct Mouse mouse;
	char* text;
	int textLen;
	
	Input() : keys(SDL_GetKeyState(0)), mods(0), mouse(), text(NULL),
		textLen(0), textMax(0){
	}
	inline void mouseClick(Uint8 button, Uint8 state){
		if(state){
			mouse.buttons |= button;
		} else {
			mouse.buttons &= ~button;
		} 
	}
	inline void mouseMove(Uint16 x, Uint16 y){
		mouse.x = x;
		mouse.y = y;
	}
	void addText(SDL_keysym& keysym){
		if(!text) return;
		switch(keysym.sym){
			case SDLK_BACKSPACE:
				if(textLen > 0){
					--textLen;
					text[textLen] = '\0';
				}
				break;
			default:
				if(textLen < textMax && keysym.unicode && !(keysym.unicode & 0xFF80)){
					text[textLen] = keysym.unicode & 0x7f;
					++textLen;
				}
				break;
		}
	}
	void setTextEntry(Uint8 enable, int maxLength){
		if((text && enable) || !(text || enable)) return;
		SDL_EnableUNICODE(enable);
		SDL_EnableKeyRepeat(enable ? 400 : 0, 30);
		if(enable){
			text = new char[maxLength+1]();
			textLen = 0;
			textMax = maxLength;
		} else {
			delete [] text;
			text = NULL;
		}	
	}
private:
	int textMax;
};

#endif
