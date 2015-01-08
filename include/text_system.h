#ifndef TEXT_SYSTEM_H_
#define TEXT_SYSTEM_H_
#include "common.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "shader.h"
#include "vertex_state.h"
#include "renderable.h"
#include "resource.h"
#include <list>

struct Font;
struct Text;

struct TextSystem : public BufferInvalidateListener {
	TextSystem(Engine& e);
	
	FT_Library& getLib();
	
	void addText(Text& t);
	void updateText(Text& t, const u32string_view& newstr, glm::ivec2 newpos);
	void delText(Text& t);

	virtual void onBufferRangeInvalidated(size_t off, size_t len); 

	~TextSystem();
private:
	GLsizei writeString(Text& t, glm::ivec2 pos, const u32string_view& str);

	FT_Library ft_lib;
	VertexState v_state;
	DynamicVertexBuffer text_buffer;
	Resource<VertShader> text_vs;
	Resource<FragShader> text_fs;
	ShaderProgram text_shader;
	BlendMode blend_mode;

	std::list<Renderable> text_renderables;
};

#endif

