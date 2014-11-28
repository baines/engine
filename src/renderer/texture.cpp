#include "texture.h"
#include "resource_system.h"
#include "render_state.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

namespace {

static GLenum get_base_fmt(GLenum sized_fmt){
	switch(sized_fmt){
		case GL_R8:
		case GL_R8_SNORM: 
		case GL_R16:
		case GL_R16_SNORM:
		case GL_R16F:
		case GL_R32F:
		case GL_R8I:
		case GL_R8UI:
		case GL_R16I:
		case GL_R16UI:
		case GL_R32I:
		case GL_R32UI:
			return GL_RED;
		
		case GL_RG8:
		case GL_RG8_SNORM:
		case GL_RG16:
		case GL_RG16_SNORM:
		case GL_RG16F:
		case GL_RG32F:
		case GL_RG8I:
		case GL_RG8UI:
		case GL_RG16I:
		case GL_RG16UI:
		case GL_RG32I:
		case GL_RG32UI:
			return GL_RG;
			
		case GL_R3_G3_B2:
		case GL_RGB4:
		case GL_RGB5:
		case GL_RGB8:
		case GL_RGB8_SNORM:
		case GL_RGB10:
		case GL_RGB12:
		case GL_RGB16_SNORM:
		case GL_RGBA2:
		case GL_RGBA4:
		case GL_SRGB8:
		case GL_RGB16F:
		case GL_R11F_G11F_B10F:
		case GL_RGB9_E5:
		case GL_RGB8I:
		case GL_RGB8UI:
		case GL_RGB16I:
		case GL_RGB16UI:
		case GL_RGB32I:
		case GL_RGB32UI:
		case GL_RGB32F:
			return GL_RGB;
					
		case GL_RGB5_A1:
		case GL_RGBA8:
		case GL_RGBA8_SNORM:
		case GL_RGB10_A2:
		case GL_RGB10_A2UI:
		case GL_RGBA12:
		case GL_RGBA16:
		case GL_SRGB8_ALPHA8:
		case GL_RGBA16F:
		case GL_RGBA32F:
		case GL_RGBA8I:
		case GL_RGBA8UI:
		case GL_RGBA16I:
		case GL_RGBA16UI:
		case GL_RGBA32I:
		case GL_RGBA32UI:
			return GL_RGBA;
	}
}
}

using namespace std;

Texture2D::Texture2D()
: id(0)
, w(0)
, h(0) {

}

Texture2D::Texture2D(GLenum fmt, GLenum int_fmt, int w, int h, const uint8_t* data)
: id(0)
, w(w)
, h(h) {

	gl.GenTextures(1, &id);
	gl.BindTexture(GL_TEXTURE_2D, id);
	gl.TexStorage2D(GL_TEXTURE_2D, 1, int_fmt, w, h);
	gl.TexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, get_base_fmt(int_fmt), fmt, data);
}

void Texture2D::loadFromResource(Engine& e, const ResourceHandle& img){
	uint8_t* pixels = stbi_load_from_memory(img.data(), img.size(), &w, &h, nullptr, 4);
	
	gl.GenTextures(1, &id);
	gl.BindTexture(GL_TEXTURE_2D, id);
	gl.TexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA, w, h);
	gl.TexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	
	stbi_image_free(pixels);
}

GLenum Texture2D::getType(void){
	return GL_TEXTURE_2D;
}

bool Texture2D::isValid(void){
	return id != 0;
}

bool Texture2D::bind(size_t tex_unit, RenderState& rs){
	if(id && id != rs.tex[tex_unit]){
		gl.ActiveTexture(GL_TEXTURE0 + tex_unit);
		gl.BindTexture(GL_TEXTURE_2D, id);
		rs.tex[tex_unit] = id;
	}
}

Texture2D::~Texture2D(){
	if(id) gl.DeleteTextures(1, &id);
}

