#pragma once

#include <vector>
#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Terrain {
public:
	Terrain() = default;
	~Terrain();
	void load(const glm::vec2 &size, const float &heightScale, const float &textureScale, std::string HeightMapLoc);
	void render();
	float getHeight(const float &worldX, const float &worldZ);
	glm::vec2 getSize() { return m_size;  };

private:
	glm::vec2 m_size;
	GLuint m_VAO, m_VBO, m_EBO;
	int m_cols, m_rows;
	std::vector<std::vector<glm::vec3>> m_vertices;
	std::vector<GLuint> m_indices;// ((rows - 1) * (cols - 1) * 2 * 3) indices in total
	std::vector<std::vector<glm::vec2>> m_texCoords;
	std::vector<std::vector<glm::vec3>> m_normals;

	void generateMesh(const glm::vec2 &size, const float &heightScale, const float &textureScale, const int &cols, const int &rows, unsigned char* heightMap);
	void bufferUpdate();
	float barryCentric(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec2 pos);
};
