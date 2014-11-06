#ifndef SHADER_ATTRIBS_H_
#define SHADER_ATTRIBS_H_
#include "common.h"
#include <vector>

struct ShaderAttribs {

	struct Attrib {
		uint32_t name_hash;
		int nelem, off, divisor;
		GLenum type;
		GLuint index;
		bool normalised, integral;
	};

	ShaderAttribs();
	
	void initAttrib(uint32_t name_hash, GLuint index);
	
	void setAttrib(const Attrib& a);
	
	const Attrib* getAttrib(uint32_t name_hash) const;
	
	const Attrib* begin() const { return attribs.data(); }
	const Attrib* end() const { return attribs.data() + attribs.size(); }
	
private:
	std::vector<Attrib> attribs;
};

#endif
