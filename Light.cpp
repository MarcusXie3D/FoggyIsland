#include "Light.h"

void Light::setShader(Shader& shader, std::string Name, GLboolean UseShader) {
	if (UseShader)
		shader.Use();
	shader.SetVector3f(Name + ".direction", m_direction);
	shader.SetVector3f(Name + ".lightColor", m_lightColor);
	shader.SetFloat(Name + ".ambientStrength", m_ambientStrength);
	shader.SetVector3f(Name + ".ambientColor", m_ambientColor);
}