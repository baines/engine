#ifndef TEXT_SYSTEM_H_
#define TEXT_SYSTEM_H_
#include "common.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "shader.h"
#include "vertex_state.h"
#include "renderable.h"
#include "resource.h"

struct TextSystem {
	TextSystem(Engine& e);
	
	FT_Library& getLib();
	
	Renderable createRenderable(...);

	~TextSystem();
private:
	FT_Library ft_lib;
	VertexState v_state;
	DynamicVertexBuffer text_buffer;
	Resource<VertShader> text_vs;
	Resource<FragShader> text_fs;
	ShaderProgram text_shader;
};

#endif

