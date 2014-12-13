#ifndef SPRITE_SYSTEM_H_
#define SPRITE_SYSTEM_H_

struct SpriteSystem {

	SpriteSystem(Engine& e);

	void addSprite(Sprite& s);
	void delSprite(Sprite& s);

private:
	std::map<Sprite*, size_t> sprite_offsets;

	VertexState v_state;

	DynamicVertexBuffer vbo;
	DynamicIndexBuffer ibo;

	Resource<VertShader> vs;
	Resource<FragShader> fs;

	ShaderProgram sprite_shader;
};

#endif
