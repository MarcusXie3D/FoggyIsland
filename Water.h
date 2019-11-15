#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include "Texture2D.h"

class Water {
public:
	Water(const glm::vec2 &size, const float &m_height);
	~Water();
	void load(std::string dudvMap, std::string normalMap, const float& scaleTex);
	void render();
	void initPassRefraction();
	void terminatePassRefraction();
	void initPassReflection();
	void terminatePassReflection();
	float getHeight() { return m_height; }
	glm::vec2 getSize() { return m_size; };

	GLuint m_VAO, m_VBO, m_FBO, m_RBO;
	Shader m_shader;
	

private:
	glm::vec2 m_size;
	float m_height;
	Texture2D m_texRefraction, m_texReflection, m_dudvMap, m_normalMap;
	float m_scaleTex;

	void init_data();
};
