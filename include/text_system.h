#ifndef TEXT_SYSTEM_H_
#define TEXT_SYSTEM_H_
#include "common.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "shader.h"
#include "vertex_state.h"
#include "renderable.h"
#include "resource.h"

struct Font;

struct TextSystem {
	TextSystem(Engine& e);
	
	FT_Library& getLib();
	
	Renderable addText(const Font& f, const glm::ivec2& pos, const string_view& str);
	void delText(Renderable& r);

	~TextSystem();
private:
	FT_Library ft_lib;
	VertexState v_state;
	DynamicVertexBuffer text_buffer;
	Resource<VertShader> text_vs;
	Resource<FragShader> text_fs;
	ShaderProgram text_shader;
	BlendMode blend_mode;
};

#endif

