#ifndef _TITLE_H_
#define _TITLE_H_
#include "gamestate.h"
#include "audio.h"
#include "text.h"
#include "util.h"
#include <algorithm>
#include <cmath>

class Title : public Gamestate {
public:
	Title(StateMgr& mgr) : Gamestate(mgr), text(160, 8, 8, 8, "Sprites: %5d", balls.size()),
		fps(160, 232, 8, 8, "FPS: %2d", 0), timer(0) { 
		for(int i = 0; i < 50; ++i){
			balls.push_back(new Ball());
		}
	}
	void update(Input& input, Uint32 delta){
		timer += delta;
		++frames;
		for(int i = 0; i < balls.size(); ++i){
			balls[i]->update();
		}
		text.update("Sprites: %5d", balls.size());
		if(timer > 500){
			fps.update("FPS: %2.2f", (frames * 1000.0f) / util::max(1.0f, (float)timer));
			timer = frames = 0;
		}
	}
	void draw(std::vector<uint32_t>& gfx){
		util::forEach(balls.begin(), balls.end(), &Sprite::draw, gfx);
		text.draw(gfx);
		fps.draw(gfx);
		//balls.push_back(new Ball());
	}
private:
	class Ball : public Sprite {
		public:
			Ball() : Sprite("nuke.png", rand()%320, rand()%240, 20, 20){
				rot = (rand()%628) / 100.0f;
				float d = rand()%2 ? 1.0f : -1.0f;
				r = (M_PI/180.0f) * (1 + (rand()%500) / 50.0f) * d;
				timer = rand()%80;
				w = h = 24 + rand()%30;
				float spd = 0.5 + (rand()%100) / 22.0f;
				xv = spd * sin(rot);
				yv = spd * cos(rot);
			}
			void update(){
				++timer;
				if((timer % 80) < 40){
					w = (w * 1.02) + 0.4;
					h = (h * 1.02) + 0.4;
				} else {
					w = (w - 0.4) / 1.02;
					h = (h - 0.4) / 1.02;
				}
				rot += r;
				if(rot > (M_PI * 2))
					rot -= (M_PI * 2);
				setRot(rot);
				x += xv;
				y += yv;
				
				if(x > 320 || x < 0) xv *= -1;
				if(y > 240 || y < 0) yv *= -1;
			}
		private:
			
			float r, rot, xv, yv;
			int timer;
	};
	std::vector<Ball*> balls;
	Text text, fps;
	int timer, frames;
};
#endif
