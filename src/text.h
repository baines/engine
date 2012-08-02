#ifndef _TEXT_H_
#define _TEXT_H_
#include "sprite.h"
#include <cstring>
#include <vector>

class Text {
public:
	Text(float x, float y, float w, float h, const char* fmt, ...) 
	: x(x), y(y), w(w), h(h), letters(), len(0){
		va_list args;
		va_start(args, fmt);
		_update(fmt, &args);
		va_end(args);
	}
	void update(const char* fmt, ...){
		va_list args;
		va_start(args, fmt);
		_update(fmt, &args);
		va_end(args);
	}
	void draw(std::vector<uint32_t>& gfx){
		for(int i = 0; i < len; ++i){
			letters[i]->draw(gfx);
		}
	}
	~Text(){
		while(!letters.empty()){
			delete letters.back();
			letters.pop_back();
		}
	}
private:
	void _update(const char* fmt, va_list* v){
		char word[256];
		len = vsnprintf(word, 256, fmt, *v);
		int letterX = x - (w * ((len-1) / 2.0f));
		
		for(int i = 0; i < len; ++i){
			if(letters.size() <= i) letters.push_back(new Sprite(getTex(), letterX, y, w, h, 16, 8));
			int j = ((int)word[i] - 32);
			letters[i]->setFrame(j % 16, j / 16);
			letters[i]->x = letterX;
			letterX += w;// * 0.8f;
		}
	}
	static Texture& getTex(){
		static Texture t("font.png");
		return t;
	}
	float x, y, w, h;
	std::vector<Sprite*> letters;
	int len, maxLength;
};
#endif
