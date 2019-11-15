#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "shader.h"

class Light {
public:
	glm::vec3 m_direction;

	glm::vec3 m_lightColor;

	GLfloat m_ambientStrength;

	glm::vec3 m_ambientColor;

	Light(glm::vec3 direction, glm::vec3 lightColor, GLfloat ambientStrength, glm::vec3 ambientColor) : m_direction(direction), m_lightColor(lightColor), m_ambientStrength(ambientStrength), m_ambientColor(ambientColor) {}

	void setShader(Shader& shader, std::string Name, GLboolean UseShader);
};
