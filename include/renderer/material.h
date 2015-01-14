#ifndef MATERIAL_H_
#define MATERIAL_H_

//TODO: multiple textures / samplers;
struct Material {

	Material() = default;
	Material(ShaderProgram& s)
		: shader(&s), texture(), sampler(){}
	Material(ShaderProgram& s, Texture& tex)
		: shader(&s), texture(&tex), sampler(){}
	Material(ShaderProgram& s, Texture& tex, Sampler& samp)
		: shader(&s), texture(&tex), sampler(&samp){}

	ShaderUniforms uniforms;
	ShaderProgram* shader;
	Texture* texture;
	Sampler* sampler;
};

#endif
