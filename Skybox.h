#pragma once

#include "shader.h"

#include <string>
#include <vector>
//#include <glad/glad.h>

class Skybox {
public:
	Skybox(Shader *shaderSkybox);
	~Skybox();
	void loadDiurnalSkybox(
		std::string right,
		std::string left,
		std::string top,
		std::string bottom,
		std::string back,
		std::string front 
		);

	void loadNocturnalSkybox(
		std::string right,
		std::string left,
		std::string top,
		std::string bottom,
		std::string back,
		std::string front
	);
	void render(const glm::mat4 &view, const glm::mat4 &projection, const float &coeDiurnal);

private:
	GLuint m_VAO, m_VBO;
	GLuint m_diurnalTexID, m_nocturnalTexID;
	Shader *m_shaderSkybox;

	void loadSkybox(
		std::string right,
		std::string left,
		std::string top,
		std::string bottom,
		std::string back,
		std::string front,
		bool const &isDiurnal
	);

	void setTexUnit();
};
