#include "text_system.h"

TextSystem::TextSystem(Engine& e)
: ft_lib(nullptr)
, text_vs(e, { "text.glslv" })
, text_fs(e, { "text.glslf" })
, text_shader(*text_vs, *text_fs) {
	assert(FT_Init_FreeType(&ft_lib) == 0);
}

FT_Library& TextSystem::getLib(){
	return ft_lib;
}

TextSystem::~TextSystem(){
	FT_Done_FreeType(ft_lib);
}

