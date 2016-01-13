#ifndef CLI_H_
#define CLI_H_
#include "common.h"
#include "game_state.h"

struct ICLI : public GameState {

	virtual bool onInput(Engine& e, int action, bool pressed) = 0;
	virtual void onText(Engine& e, const char* text) = 0;
	virtual void onStateChange(Engine& e, bool activated) = 0;
	virtual void onResize(Engine& e, int w, int h) = 0;
	virtual void update(Engine& e, uint32_t delta) = 0;
	virtual void draw(IRenderer& r) = 0;
	virtual void toggle(void) = 0;
	virtual void echo(const StrRef& str) = 0;
	virtual void echo(const std::initializer_list<StrRef> str) = 0;
	virtual void printf(const char* fmt, ...) __attribute__ ((format (printf, 2, 3))) = 0;
	virtual void printVarInfo(const CVar& cvar) = 0;

	virtual ~ICLI(){}
};

#endif

