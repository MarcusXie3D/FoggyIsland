#include "Fog.h"

void Fog::setShader(Shader& shader, std::string Name, GLboolean UseShader) {
	if (UseShader)
		shader.Use();
	shader.SetFloat(Name + ".Density", m_Density);
	shader.SetVector3f(Name + ".Color", m_Color);
}