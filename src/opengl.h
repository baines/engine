#ifndef _GL_FN
	#define _GL_FN(type, name, args) \
		typedef type (APIENTRY * name##_p) args;
	#define UNDEF_GL_FN
#endif
_GL_FN(GLenum, GetError, (void))
_GL_FN(void, FrontFace, (GLenum  mode))
_GL_FN(const GLubyte*, GetString, (GLenum name))
_GL_FN(void, GenTextures, (GLsizei n, GLuint *textures))
_GL_FN(void, BindTexture, (GLenum target, GLuint texture))
_GL_FN(void, TexParameteri, (GLenum target, GLenum pname, GLint param))
_GL_FN(void, TexImage2D, (GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels))
_GL_FN(void, Clear, (GLbitfield mask))
_GL_FN(void, MatrixMode, (GLenum mode))
_GL_FN(void, Viewport, (GLint x, GLint y, GLsizei width, GLsizei height))
_GL_FN(void, LoadIdentity, (void))
_GL_FN(void, Enable, (GLenum cap))
_GL_FN(void, Ortho, (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val))
_GL_FN(void, BlendFunc, (GLenum sfactor, GLenum dfactor))
_GL_FN(void, DrawElements, (GLenum mode, GLsizei  count,  GLenum  type,  const GLvoid *  indices))
_GL_FN(void, VertexAttribPointerARB, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer))
_GL_FN(void, ProgramStringARB, (GLenum target, GLenum format, GLsizei len, const GLvoid *string))
_GL_FN(void, GenProgramsARB, (GLsizei n, GLuint *programs))
_GL_FN(void, BindProgramARB, (GLenum target, GLuint program))
_GL_FN(void, EnableVertexAttribArrayARB, (GLuint index))
_GL_FN(void, BindBuffer, (GLenum target, GLuint buffer))
_GL_FN(void, BufferData, (GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage))
_GL_FN(void, GetIntegerv, (GLenum  pname,  GLint *  params))
#ifdef UNDEF_GL_FN
	#undef _GL_FN
	#undef UNDEF_GL_FN
#endif
