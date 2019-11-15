#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "shader.h"

class Fog {
public:
	float m_Density;
	glm::vec3 m_Color;

	Fog(float Density, glm::vec3 Color) : m_Density(Density), m_Color(Color) {}

	void setShader(Shader& shader, std::string Name, GLboolean UseShader);
};
