#ifndef RENDERABLE_H_
#define RENDERABLE_H_
#include "common.h"
#include "gl_context.h"
#include "blend_mode.h"
#include "shader_uniforms.h"
#include <array>

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
	
	bool usesSameState(const Renderable& o){

		bool uniforms_compat = uniforms && o.uniforms
			? *uniforms == *o.uniforms
			: !uniforms && !o.uniforms;

		return prim_type == o.prim_type
		    && blend_mode == o.blend_mode
		    && vertex_state == o.vertex_state
		    && shader == o.shader
		    && textures == o.textures
		    && samplers == o.samplers
			&& uniforms_compat;
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

