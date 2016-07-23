#ifndef TEXT_SYSTEM_PRIVATE_H_
#define TEXT_SYSTEM_PRIVATE_H_
#include "text_system.h"
#include "blend_mode.h"
#include "shader.h"
#include "vertex_state.h"
#include "resource.h"
#include "vertex_buffer.h"
#include <vector>

struct FT_LibraryRec_;

//extern template class std::vector<Text*>;

struct TextSystem : public ITextSystem, public BufferInvalidateListener {
	TextSystem(Engine& e);
	
	FT_LibraryRec_*& getLib();
	
	void addText(Text& t);
	void updateText(Text& t, const StrRef32& newstr, int x, int y);
	void delText(Text& t);

	void onBufferRangeInvalidated(size_t off, size_t len); 

	~TextSystem();
private:
	size_t writeString(Text& t, vec2i pos, const StrRef32& str);

	FT_LibraryRec_* ft_lib;
	VertexState v_state;
	DynamicVertexBuffer text_buffer;
	Resource<VertShader> text_vs;
	Resource<FragShader> text_fs;
	ShaderProgram text_shader;
	BlendMode blend_mode;
	std::vector<Text*> texts;
};

#endif

