#include "texture.h"
#include "resource_system.h"
#include "render_state.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

using namespace std;

Texture2D::Texture2D()
: id(0){

}

void Texture2D::load(const ResourceHandle& img){
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

