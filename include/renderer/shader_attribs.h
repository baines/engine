#ifndef SHADER_ATTRIBS_H_
#define SHADER_ATTRIBS_H_
#include "common.h"
#include <GL/gl.h>
#include <vector>

enum AttribFlags : uint32_t {
	ATR_NORM       = 0x01,
	ATR_INT        = 0x02,
	ATR_INSTANCED  = 0x04,
};

struct ShaderAttribs {
	struct Attrib {
		uint32_t name_hash;
		GLint index;
		
		GLenum type;
		int nelem, off;
		uint32_t flags;
		
		bool operator==(uint32_t h) const { return name_hash == h; }
	};
	
	ShaderAttribs();
	
	void initAttrib(uint32_t name_hash, GLint index);
	void setAttribFormat(uint32_t hash, GLenum type, int nelem, int off, uint32_t flags);
	bool bind(uint32_t hash, GLuint index, GLuint stride) const;
	void clear();

	bool containsAttrib(uint32_t hash, GLint at_index = -1) const;
	
	const Attrib* begin() const;
	const Attrib* end() const;
	
private:
	std::vector<Attrib> attribs;
};

#endif
