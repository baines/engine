#ifndef RENDERABLE_H_
#define RENDERABLE_H_
#include "texture.h"
#include "sampler.h"
#include "vertex_state.h"
#include "shader.h"
#include "shader_uniforms.h"
#include "blend_mode.h"
#include "util.h"

struct RCount { GLsizei value; };
struct RType  { GLenum  value; };
struct ROff   { GLint   value; };

struct Renderable {

	void set(VertexState* vs){ vertex_state = vs; }
	void set(ShaderProgram* sp){ shader = sp; }
	void set(ShaderUniforms* su){ uniforms = su; }
	void set(BlendMode& bm){ blend_mode = bm; }
	void set(RCount c){ count = c.value; }
	void set(RType t){ prim_type = t.value; }
	void set(ROff o){ offset = o.value; }

	template<class T, class... Args>
	void set(T&& t, Args&&... args){
		set(std::forward<T>(t));
		set(std::forward<Args>(args)...);
	}

	Renderable()
	: textures()
	, samplers()
	, vertex_state(nullptr)
	, shader(nullptr)
	, uniforms(nullptr)
	, blend_mode()
	, prim_type(GL_TRIANGLES)
	, count(0)
	, offset(0) {
	
	}
	
	Renderable(const Renderable&) = default;
	Renderable(Renderable&&) = default;
	Renderable& operator=(Renderable&&) = default;
	
	template<class... Args>
	Renderable(Args&&... args) : Renderable() {
		set(std::forward<Args>(args)...);
	}
	
	std::array<const Texture*, 8> textures;
	std::array<const Sampler*, 8> samplers;
	
	VertexState*    vertex_state;
	ShaderProgram*  shader;
	ShaderUniforms* uniforms;
	
	BlendMode blend_mode;
	GLenum    prim_type;
	GLsizei   count;
	GLint     offset;
	
};

#endif

