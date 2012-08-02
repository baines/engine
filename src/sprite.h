#ifndef _SPRITE_H_
#define _SPRITE_H_
#include "texture.h"
#include "renderer.h"
#include "util.h"
#include <cmath>

class Sprite {
public:
	Sprite(const Texture& tex, float x, float y, float w, float h, int xSegs = 1, int ySegs = 1)
		: x(x), y(y), w(w), h(h), xSegs(xSegs), ySegs(ySegs){	
		Renderer::get().addQuad(x, y, w, h, &index);
		vert(0).texID = tex.id;
		vert(0).rot3 = vert(1).rot3 = vert(2).rot3 = vert(3).rot3 = sin(M_PI / 18.0f);
		vert(0).rot4 = vert(1).rot4 = vert(2).rot4 = vert(3).rot4 = cos(M_PI / 18.0f);
		setTextureBounds(0, 0, 1.0f/float(xSegs), 1.0f/float(ySegs));
		setx(x);
		sety(y);
		setRot(0);
	}
	virtual ~Sprite(){ Renderer::get().delQuad(index); }
	void setTextureBounds(float x1, float y1, float x2, float y2){
		vert(0).textx = vert(3).textx = x1 * 128;
		vert(1).textx = vert(2).textx = x2 * 128;
		vert(0).texty = vert(1).texty = y1 * 128;
		vert(2).texty = vert(3).texty = y2 * 128;
	}
	void setRot(float angle){
		float _w = w / 2.0f, _h = h / 2.0f;
		vert(0).rot3 = vert(1).rot3 = vert(2).rot3 = vert(3).rot3 = sin(angle);
		vert(0).rot1 = vert(3).rot1 = _w;
		vert(0).rot2 = vert(1).rot2 = _h;
		vert(1).rot1 = vert(2).rot1 = -_w;
		vert(2).rot2 = vert(3).rot2 = -_h;
		vert(0).rot4 = vert(1).rot4 = vert(2).rot4 = vert(3).rot4 = cos(angle);
	}
	virtual void draw(std::vector<uint32_t>& gfx){
		if(vert(0).vertx != (x - w / 2.0f) || vert(1).vertx != (x + w / 2.0f)) setx(x);
		if(vert(0).verty != (y - h / 2.0f) || vert(2).verty != (y + h / 2.0f)) sety(y);
		for(int i = 0; i < 4; ++i) gfx.push_back(index+i);
	}
	void setFrame(int x, int y){
		float _x = util::max(util::min(x, xSegs - 1), 0);
		float _y = util::max(util::min(y, ySegs - 1), 0);
		setTextureBounds(_x/float(xSegs), _y/float(ySegs), (_x+1.0f)/float(xSegs), (_y+1.0f)/float(ySegs));
	}
	static bool collision(Sprite* one, Sprite* two){
		if((one->x + one->w / 2.0f < two->x - two->w / 2.0f) ||
		   (one->x - one->w / 2.0f > two->x + two->w / 2.0f) ||
		   (one->y + one->h / 2.0f < two->y - two->w / 2.0f) ||
		   (one->y - one->h / 2.0f > two->y + two->w / 2.0f)) {
			return false;
		} else {
			return true;
		}
	}
	float x, y, w, h;
private:
	inline Vertex& vert(int x){
		static Vec<Vert4>& v = Renderer::get().gfx;
		return v[index / 4].v[x];
	}	
	inline void setx(float val){
		vert(0).vertx = vert(3).vertx = val - w / 2.0f;
		vert(1).vertx = vert(2).vertx = val + w / 2.0f;
	}
	inline void sety(float val){
		vert(0).verty = vert(1).verty = val - h / 2.0f;
		vert(2).verty = vert(3).verty = val + h / 2.0f;
	}
	uint32_t index;
	int xSegs, ySegs;
};
#endif

