#ifndef TEXT_SYSTEM_H_
#define TEXT_SYSTEM_H_
#include "common.h"

struct FT_LibraryRec_;

struct ITextSystem {
	virtual FT_LibraryRec_*& getLib() = 0;
	virtual void addText(Text& t) = 0;
	virtual void updateText(Text& t, const StrRef32& newstr, int x, int y) = 0;
	virtual void delText(Text& t) = 0;
	virtual ~ITextSystem(){}
};

#endif

