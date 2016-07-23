#include "texture.h"
#include "sampler.h"
#include "resource_system.h"
#include "render_state.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

namespace {

//TODO: use ARB_internalformat_query if available
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
			
		/* extensions */
		
		case GL_COMPRESSED_RED_RGTC1:
		case GL_COMPRESSED_SIGNED_RED_RGTC1:
			return GL_RED; 
			
		case GL_COMPRESSED_RG_RGTC2:
		case GL_COMPRESSED_SIGNED_RG_RGTC2:
			return GL_RG;
			
		/* legacy */
		
		case GL_ALPHA4:
		case GL_ALPHA8:
		case GL_ALPHA12:
		case GL_ALPHA16:
			return GL_ALPHA;	
		case GL_LUMINANCE4:
		case GL_LUMINANCE8:
		case GL_LUMINANCE12:
		case GL_LUMINANCE16:
			return GL_LUMINANCE;
			
		case GL_LUMINANCE4_ALPHA4:
		case GL_LUMINANCE6_ALPHA2:
		case GL_LUMINANCE8_ALPHA8:
		case GL_LUMINANCE12_ALPHA4:
		case GL_LUMINANCE16_ALPHA16:
			return GL_LUMINANCE_ALPHA;
		
		case GL_INTENSITY4:
		case GL_INTENSITY8:
		case GL_INTENSITY12:
		case GL_INTENSITY16:
			return GL_INTENSITY;
			
		default: {
			log(logging::warn, "Unknown internal format %#x.", sized_fmt);
			return GL_RGBA;
		}
	}
}

void texture2d_init(GLuint& id, GLenum type, GLenum int_fmt, int w, int h, const void* data){

	gl.GenTextures(1, &id);
	gl.BindTexture(GL_TEXTURE_2D, id);
	if(gl.TexStorage2D){
		gl.TexStorage2D(GL_TEXTURE_2D, 1, int_fmt, w, h);
		gl.TexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, get_base_fmt(int_fmt), type, data);
	} else {
		gl.TexImage2D(GL_TEXTURE_2D, 0, get_base_fmt(int_fmt), w, h, 0, get_base_fmt(int_fmt), type, data);
		gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);	
	}
}

}

using namespace std;

Texture2D::Texture2D()
: id(0)
, w(0)
, h(0) {

}

Texture2D::Texture2D(MemBlock img)
: id(0)
, w(0)
, h(0) {
	uint8_t* pixels = stbi_load_from_memory(img.ptr, img.size, &w, &h, nullptr, 4);
	
	texture2d_init(id, GL_UNSIGNED_BYTE, GL_RGBA8, w, h, pixels);
	
	stbi_image_free(pixels);
}

Texture2D::Texture2D(GLenum type, GLenum int_fmt, int w, int h, const void* data)
: id(0)
, w(w)
, h(h) {
	texture2d_init(id, type, int_fmt, w, h, data);
}

Texture2D& Texture2D::operator=(Texture2D&& other){
	std::swap(id, other.id);
	std::swap(w, other.w);
	std::swap(h, other.h);
	return *this;
}

bool Texture2D::setSwizzle(const Array<GLint, 4>& swizzle){
	if(!(gl.version >= 33
	|| gl.hasExtension("EXT_texture_swizzle") 
	|| gl.hasExtension("ARB_texture_swizzle"))){
		log(logging::error, "Can't set swizzle: not supported by OpenGL driver.");
		return false;
	}
	//XXX: assume texture is already bound
	gl.TexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle.data());
	return true;
}

GLenum Texture2D::getType(void) const {
	return GL_TEXTURE_2D;
}

bool Texture2D::isValid(void) const {
	return id != 0;
}

std::tuple<int, int> Texture2D::getSize() const {
	return std::make_tuple(w, h);
}

bool Texture2D::bind(size_t tex_unit, RenderState& rs) const {
	if(id && id != rs.tex[tex_unit]){
		if(rs.active_tex != tex_unit){
			gl.ActiveTexture(GL_TEXTURE0 + tex_unit);
			rs.active_tex = tex_unit;
		}
		gl.BindTexture(GL_TEXTURE_2D, id);
		rs.tex[tex_unit] = id;
	}
	return true;
}

void Texture2D::onGLContextRecreate(){
	// If this is called, then this is a texture not managed by a Resource<>..
	// Whatever is managing it instead is responsible for recreating it.
	// The id is set to 0 so that the destuctor doesn't attempt to delete an invalid id.
	id = 0;
}

Texture2D::~Texture2D(){
	if(id && gl.initialized()) gl.DeleteTextures(1, &id);
}

Sampler::Sampler()
: params()
, id(0){
	if(gl.GenSamplers) gl.GenSamplers(1, &id);
}

Sampler::Sampler(std::initializer_list<Param> params)
: params()
, id(0) {
	if(gl.GenSamplers) gl.GenSamplers(1, &id);
	
	for(auto& p : params){
		setParam(p);
	}
}

void Sampler::setParam(const Param& p){
	setParam(p.key, p.val);
}

void Sampler::setParam(GLenum key, GLint val){
	params[key] = val;
	if(gl.SamplerParameteri){
		gl.SamplerParameteri(id, key, val);
	} else {
		//TODO: fallback if sampler_objects unsupported.
	}
}

void Sampler::bind(size_t tex_unit, RenderState& rs) const {
	if(gl.BindSampler && rs.samp[tex_unit] != id){
		gl.BindSampler(tex_unit, id);
		rs.samp[tex_unit] = id;
	}
}

void Sampler::onGLContextRecreate(){
	if(gl.GenSamplers) gl.GenSamplers(1, &id);
	for(auto& p : params){
		setParam(p.first, p.second);
	}
}

Sampler::~Sampler(){
	if(gl.DeleteSamplers && id){
		gl.DeleteSamplers(1, &id);
	}
}
