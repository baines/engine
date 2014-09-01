#ifndef GLFUNC
	#define GLFUNC(type, name, args, ...) \
		typedef type (APIENTRY * name##_p) args;
	#define UNDEF_GLFUNC
#endif

#define MIN_GL_VERSION 2.0

GLFUNC(void, Clear, (GLbitfield flags))

GLFUNC(void, DrawArrays, (GLenum, GLint, GLsizei))
GLFUNC(void, DrawElements, (GLenum, GLsizei, GLenum, const GLvoid*))
GLFUNC(void, GenFrameBuffers, (GLsizei, GLuint*), ARBCORE | EXT | 30, "framebuffer_object")
GLFUNC(void, BindFramebuffer, (GLenum, GLuint), ARBCORE | EXT | 30, "framebuffer_object")
GLFUNC(void, FramebufferTexture2D, (GLenum, GLenum, GLuint, GLint), ARBCORE | EXT | 30, "framebuffer_object")

GLFUNC(void, GenBuffers, (GLsizei, GLuint*))
GLFUNC(void, BindBuffer, (GLenum, GLuint))
GLFUNC(void, BufferData, (GLenum, GLsizeiptr, const GLvoid*, GLenum))
GLFUNC(void, InvalidateBufferData, (GLuint), ARBCORE | 43, "invalidate_subdata")

GLFUNC(void, GenVertexArrays, (GLsizei, GLuint*), ARBCORE | 30, "vertex_array_object")
GLFUNC(void, BindVertexArray, (GLuint), ARBCORE | 30, "vertex_array_object")
GLFUNC(void, VertexAttribFormat, (GLuint, GLint, GLenum, GLboolean, GLuint), ARBCORE | 43, "vertex_attrib_binding")
GLFUNC(void, VertexAttribIFormat, (GLuint, GLint, GLenum, GLuint), ARBCORE | 43, "vertex_attrib_binding")
GLFUNC(void, VertexAttribBinding, (GLuint, GLuint), ARBCORE | 43, "vertex_attrib_binding")
GLFUNC(void, VertexAttribDivisor, (GLuint, GLuint), ARB | 33, "instanced_arrays")
GLFUNC(void, EnableVertexAttribArray, (GLuint))
GLFUNC(void, DisableVertexAttribArray, (GLuint))

GLFUNC(void, GenTextures, (GLsizei, GLuint*))
GLFUNC(void, BindTexture, (GLenum, GLuint))
GLFUNC(void, ActiveTexture, (GLenum))
GLFUNC(void, TexStorage2D, (GLenum, GLsizei, GLenum, GLsizei, GLsizei), ARBCORE | EXT | 42, "texture_storage")
GLFUNC(void, TexSubImage2D, (GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid*))
GLFUNC(void, DeleteTextures, (GLsizei, const GLuint*))

GLFUNC(void, GenSamplers, (GLsizei, GLuint*), ARBCORE | 33, "sampler_objects")
GLFUNC(void, BindSampler, (GLuint, GLuint), ARBCORE | 33, "sampler_objects")
GLFUNC(void, SamplerParameteri, (GLuint, GLenum, GLint), ARBCORE | 33, "sampler_objects")

GLFUNC(GLuint, CreateShader, (GLenum))
GLFUNC(void, ShaderSource, (GLuint, GLsizei, const GLchar**, const GLint*))
GLFUNC(void, AttachShader, (GLuint, GLuint))
GLFUNC(void, DetachShader, (GLuint, GLuint))
GLFUNC(void, CompileShader, (GLuint))
GLFUNC(void, DeleteShader, (GLuint))
GLFUNC(void, GetShaderiv, (GLuint, GLenum, GLint*))
GLFUNC(void, GetShaderInfoLog, (GLuint, GLsizei, GLsizei*, GLchar*))
GLFUNC(void, GetAttachedShaders, (GLuint, GLsizei, GLsizei*, GLuint*))
GLFUNC(GLuint, CreateProgram, (void))
GLFUNC(void, LinkProgram, (GLuint))
GLFUNC(void, GetProgramiv, (GLuint, GLenum, GLint*))
GLFUNC(void, GetProgramInfoLog, (GLuint, GLsizei, GLsizei*, GLchar*))
GLFUNC(void, UseProgram, (GLuint))
GLFUNC(void, DeleteProgram, (GLuint))

GLFUNC(void, Uniform1fv, (GLint, GLsizei, const GLfloat*))
GLFUNC(void, Uniform2fv, (GLint, GLsizei, const GLfloat*))
GLFUNC(void, Uniform3fv, (GLint, GLsizei, const GLfloat*))
GLFUNC(void, Uniform4fv, (GLint, GLsizei, const GLfloat*))

GLFUNC(void, Uniform1iv, (GLint, GLsizei, const GLint*))
GLFUNC(void, Uniform2iv, (GLint, GLsizei, const GLint*))
GLFUNC(void, Uniform3iv, (GLint, GLsizei, const GLint*))
GLFUNC(void, Uniform4iv, (GLint, GLsizei, const GLint*))

GLFUNC(void, Uniform1uiv, (GLint, GLsizei, const GLuint*), 30)
GLFUNC(void, Uniform2uiv, (GLint, GLsizei, const GLuint*), 30)
GLFUNC(void, Uniform3uiv, (GLint, GLsizei, const GLuint*), 30)
GLFUNC(void, Uniform4uiv, (GLint, GLsizei, const GLuint*), 30)

GLFUNC(void, UniformMatrix2fv, (GLint, GLsizei, GLboolean, const GLfloat*))
GLFUNC(void, UniformMatrix3fv, (GLint, GLsizei, GLboolean, const GLfloat*))
GLFUNC(void, UniformMatrix4fv, (GLint, GLsizei, GLboolean, const GLfloat*))

GLFUNC(void, UniformMatrix2x3fv, (GLint, GLsizei, GLboolean, const GLfloat*), 21)
GLFUNC(void, UniformMatrix3x2fv, (GLint, GLsizei, GLboolean, const GLfloat*), 21)
GLFUNC(void, UniformMatrix2x4fv, (GLint, GLsizei, GLboolean, const GLfloat*), 21)
GLFUNC(void, UniformMatrix4x2fv, (GLint, GLsizei, GLboolean, const GLfloat*), 21)
GLFUNC(void, UniformMatrix3x4fv, (GLint, GLsizei, GLboolean, const GLfloat*), 21)
GLFUNC(void, UniformMatrix4x3fv, (GLint, GLsizei, GLboolean, const GLfloat*), 21)

#ifdef UNDEF_GLFUNC
	#undef GLFUNC
	#undef UNDEF_GLFUNC
#endif