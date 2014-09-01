
struct Camera {

	// glm matrices, frustum planes

	void setFoV(int fov);
	
	glm::mat4& getMatrix();
	
	// rotate, translate e.t.c functions perhaps
	
	bool isPointVisible();
	bool isSphereVisible();
	bool isBoxVisible();

};
