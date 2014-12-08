#ifndef GLFUNC
	#define GLFUNC(type, name, args, ...) \
		typedef type (APIENTRY * name##_p) args;
	#define UNDEF_GLFUNC
#endif

#define MIN_GL_VERSION 2.0

GLFUNC(void, Clear, (GLbitfield flags))
GLFUNC(void, Viewport, (GLint, GLint, GLsizei, GLsizei))
GLFUNC(void, Enable, (GLenum))
GLFUNC(void, Disable, (GLenum))

GLFUNC(void, DrawArrays, (GLenum, GLint, GLsizei))
GLFUNC(void, DrawElements, (GLenum, GLsizei, GLenum, const GLvoid*))
GLFUNC(void, GenFramebuffers, (GLsizei, GLuint*), ARBCORE | EXT | 30, "framebuffer_object")
GLFUNC(void, BindFramebuffer, (GLenum, GLuint), ARBCORE | EXT | 30, "framebuffer_object")
GLFUNC(void, FramebufferTexture2D, (GLenum, GLenum, GLuint, GLint), ARBCORE | EXT | 30, "framebuffer_object")

GLFUNC(void, GenBuffers, (GLsizei, GLuint*))
GLFUNC(void, BindBuffer, (GLenum, GLuint))
GLFUNC(void, BufferData, (GLenum, GLsizeiptr, const GLvoid*, GLenum))
GLFUNC(void, BufferSubData, (GLenum, GLintptr, GLsizeiptr, const GLvoid*))
GLFUNC(void, DeleteBuffers, (GLsizei, GLuint*))
GLFUNC(void, InvalidateBufferData, (GLuint), OPTIONAL | ARBCORE | 43, "invalidate_subdata")
GLFUNC(void*, MapBuffer, (GLenum, GLenum))
GLFUNC(void*, MapBufferRange, (GLenum, GLintptr, GLsizeiptr, GLbitfield), OPTIONAL | ARBCORE | 30, "map_buffer_range")
GLFUNC(GLboolean, UnmapBuffer, (GLenum))

GLFUNC(void, GenVertexArrays, (GLsizei, GLuint*), ARBCORE | 30, "vertex_array_object")
GLFUNC(void, DeleteVertexArrays, (GLsizei, GLuint*), ARBCORE | 30, "vertex_array_object")
GLFUNC(void, BindVertexArray, (GLuint), ARBCORE | 30, "vertex_array_object")
GLFUNC(void, BindVertexBuffer, (GLuint, GLuint, GLintptr, GLsizei), ARBCORE | 43, "vertex_attrib_binding")
GLFUNC(void, VertexAttribFormat, (GLuint, GLint, GLenum, GLboolean, GLuint), ARBCORE | 43, "vertex_attrib_binding")
GLFUNC(void, VertexAttribIFormat, (GLuint, GLint, GLenum, GLuint), ARBCORE | 43, "vertex_attrib_binding")
GLFUNC(void, VertexAttribBinding, (GLuint, GLuint), ARBCORE | 43, "vertex_attrib_binding")
GLFUNC(void, VertexAttribDivisor, (GLuint, GLuint), OPTIONAL | ARB | 33, "instanced_arrays")
GLFUNC(void, EnableVertexAttribArray, (GLuint))
GLFUNC(void, DisableVertexAttribArray, (GLuint))

GLFUNC(void, GenTextures, (GLsizei, GLuint*))
GLFUNC(void, BindTexture, (GLenum, GLuint))
GLFUNC(void, ActiveTexture, (GLenum))
GLFUNC(void, TexStorage2D, (GLenum, GLsizei, GLenum, GLsizei, GLsizei), ARBCORE | EXT | 42, "texture_storage")
GLFUNC(void, TexSubImage2D, (GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid*))
GLFUNC(void, TexParameteriv, (GLenum, GLenum, const GLint*))
GLFUNC(void, DeleteTextures, (GLsizei, const GLuint*))

GLFUNC(void, GenSamplers, (GLsizei, GLuint*), OPTIONAL | ARBCORE | 33, "sampler_objects")
GLFUNC(void, BindSampler, (GLuint, GLuint), OPTIONAL | ARBCORE | 33, "sampler_objects")
GLFUNC(void, SamplerParameteri, (GLuint, GLenum, GLint), OPTIONAL | ARBCORE | 33, "sampler_objects")

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

GLFUNC(void, GetActiveAttrib, (GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*))
GLFUNC(GLint, GetAttribLocation, (GLuint, const GLchar*))
GLFUNC(void, GetActiveUniform, (GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*))
GLFUNC(GLint, GetUniformLocation, (GLuint, const GLchar*))

GLFUNC(void, GetUniformfv, (GLuint, GLint, GLfloat*))
GLFUNC(void, GetUniformiv, (GLuint, GLint, GLint*))
GLFUNC(void, GetUniformuiv, (GLuint, GLint, GLuint*), OPTIONAL | 30)

GLFUNC(void, GetnUniformfv, (GLuint, GLint, GLsizei, GLfloat*), OPTIONAL | ARB | EXT | 45, "robustness")
GLFUNC(void, GetnUniformiv, (GLuint, GLint, GLsizei, GLint*), OPTIONAL | ARB | EXT | 45, "robustness")
GLFUNC(void, GetnUniformuiv, (GLuint, GLint, GLsizei, GLuint*), OPTIONAL | ARB | 45, "robustness")

GLFUNC(void, Uniform1fv, (GLint, GLsizei, const GLfloat*))
GLFUNC(void, Uniform2fv, (GLint, GLsizei, const GLfloat*))
GLFUNC(void, Uniform3fv, (GLint, GLsizei, const GLfloat*))
GLFUNC(void, Uniform4fv, (GLint, GLsizei, const GLfloat*))

GLFUNC(void, Uniform1iv, (GLint, GLsizei, const GLint*))
GLFUNC(void, Uniform2iv, (GLint, GLsizei, const GLint*))
GLFUNC(void, Uniform3iv, (GLint, GLsizei, const GLint*))
GLFUNC(void, Uniform4iv, (GLint, GLsizei, const GLint*))

GLFUNC(void, Uniform1uiv, (GLint, GLsizei, const GLuint*), OPTIONAL | 30)
GLFUNC(void, Uniform2uiv, (GLint, GLsizei, const GLuint*), OPTIONAL | 30)
GLFUNC(void, Uniform3uiv, (GLint, GLsizei, const GLuint*), OPTIONAL | 30)
GLFUNC(void, Uniform4uiv, (GLint, GLsizei, const GLuint*), OPTIONAL | 30)

GLFUNC(void, UniformMatrix2fv, (GLint, GLsizei, GLboolean, const GLfloat*))
GLFUNC(void, UniformMatrix3fv, (GLint, GLsizei, GLboolean, const GLfloat*))
GLFUNC(void, UniformMatrix4fv, (GLint, GLsizei, GLboolean, const GLfloat*))

GLFUNC(void, UniformMatrix2x3fv, (GLint, GLsizei, GLboolean, const GLfloat*), 21)
GLFUNC(void, UniformMatrix3x2fv, (GLint, GLsizei, GLboolean, const GLfloat*), 21)
GLFUNC(void, UniformMatrix2x4fv, (GLint, GLsizei, GLboolean, const GLfloat*), 21)
GLFUNC(void, UniformMatrix4x2fv, (GLint, GLsizei, GLboolean, const GLfloat*), 21)
GLFUNC(void, UniformMatrix3x4fv, (GLint, GLsizei, GLboolean, const GLfloat*), 21)
GLFUNC(void, UniformMatrix4x3fv, (GLint, GLsizei, GLboolean, const GLfloat*), 21)

GLFUNC(void, BlendFuncSeparate, (GLenum, GLenum, GLenum, GLenum))
GLFUNC(void, BlendEquationSeparate, (GLenum, GLenum))

GLFUNC(void, DebugMessageCallback, (GLDEBUGPROC, void*), OPTIONAL | ARB | 43, "debug_output")

#ifdef UNDEF_GLFUNC
	#undef GLFUNC
	#undef UNDEF_GLFUNC
#endif
