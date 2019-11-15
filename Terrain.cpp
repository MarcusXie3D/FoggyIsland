#include "Terrain.h"
#include <SOIL.h>

Terrain::~Terrain()
{
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_EBO);
	glDeleteBuffers(1, &m_VBO);
}

void Terrain::load(const glm::vec2 &size, const float &heightScale, const float &textureScale, std::string HeightMapLoc) {
	unsigned char* heightMap = SOIL_load_image(HeightMapLoc.c_str(), &m_cols, &m_rows, 0, SOIL_LOAD_L);
	generateMesh(size, heightScale, textureScale, m_cols, m_rows, heightMap);
	bufferUpdate();
}

void Terrain::render() {
	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

float Terrain::getHeight(const float &worldX, const float &worldZ) {
	GLfloat terrainX = worldX + 0.5 * m_size.x;
	GLfloat terrainZ = worldZ + 0.5 * m_size.y;
	GLfloat gridSquareSizeX = m_size.x / (m_cols - 1);
	GLfloat gridSquareSizeZ = m_size.y / (m_rows - 1);
	GLuint gridX = terrainX / gridSquareSizeX;
	GLuint gridZ = terrainZ / gridSquareSizeZ;

	// boundary check
	if (gridX > (m_cols - 1) || gridZ > (m_rows - 1) || gridX < 0.f || gridZ < 0.f)
		return 0.f;

	GLfloat xCoordSquare = fmod(terrainX, gridSquareSizeX) / gridSquareSizeX; 
	GLfloat zCoordSquare = fmod(terrainZ, gridSquareSizeZ) / gridSquareSizeZ; 
	GLfloat height = 0.0f;
	if (xCoordSquare <= 1 - zCoordSquare) // Left triangle // might be zCoordSquare, check here if it goes wrong
	{
		height = barryCentric(glm::vec3(0, m_vertices[gridZ][gridX].y, 0),
			glm::vec3(1, m_vertices[gridZ][gridX + 1].y, 0),
			glm::vec3(0, m_vertices[gridZ + 1][gridX].y, 1),
			glm::vec2(xCoordSquare, zCoordSquare)
		);
	}
	else // Right triangle
	{
		height = barryCentric(glm::vec3(1, m_vertices[gridZ][gridX + 1].y, 0),
			glm::vec3(1, m_vertices[gridZ + 1][gridX + 1].y, 1),
			glm::vec3(0, m_vertices[gridZ + 1][gridX].y, 1),
			glm::vec2(xCoordSquare, zCoordSquare)
		);
	}

	return height;
}

void Terrain::generateMesh(const glm::vec2 &size, const float &heightScale, const float &textureScale,const int &cols, const int &rows, unsigned char* heightMap) {
	m_size = size;

	/************************** Generate Vertices and TexCoords ************************/ 
	m_vertices = std::vector<std::vector<glm::vec3>>(rows, std::vector<glm::vec3>(cols));
	m_texCoords = std::vector<std::vector<glm::vec2>>(rows, std::vector<glm::vec2>(cols));
	for (GLuint i = 0; i < rows; i++) {
		for (GLuint j = 0; j < cols; j++) {
			float luminance = heightMap[i * cols + j];
			float height = luminance / 255.f * heightScale;
			float scaleCol = j / (cols - 1.f) - 0.5f;
			float scaleRow = i / (rows - 1.f) - 0.5f;
			m_vertices[i][j] = glm::vec3(scaleCol * size.x, height, scaleRow * size.y);
			m_texCoords[i][j] = glm::vec2(scaleCol * textureScale, scaleRow * textureScale);
		}
	}


	/************************** Generate Indices ************************/
	// iterate each square, which is formed by 2 triangles
	for (GLuint i = 0; i < rows - 1; i++) {
		for (GLuint j = 0; j < cols - 1; j++) {
			/*
				0-------------2
				|           /   0
				| upper  /   /  |
				|     /   /     |
				|  /   /  lower |
				1   /           |
				 1--------------2
			*/
			// upper triangle, in counter clockwise
			m_indices.push_back(i * cols + j);
			m_indices.push_back((i + 1) * cols + j);
			m_indices.push_back(i * cols + j + 1);

			// lower triangle, in counter clockwise
			m_indices.push_back(i * cols + j + 1);
			m_indices.push_back((i + 1) * cols + j);
			m_indices.push_back((i + 1) * cols + j + 1);
		}
	}


	/************************** Generate Normals ************************/
	// 1. calculate per-face normals
	std::vector<std::vector<glm::vec3>> m_faceNormals[2];
	m_faceNormals[0] = std::vector<std::vector<glm::vec3>>(rows - 1, std::vector<glm::vec3>(cols - 1));
	m_faceNormals[1] = std::vector<std::vector<glm::vec3>>(rows - 1, std::vector<glm::vec3>(cols - 1));
	for (GLuint i = 0; i < rows - 1; i++) {
		for (GLuint j = 0; j < cols - 1; j++) {
			/*
				0-------------2
				|           /   0
				| upper  /   /  |
				|     /   /     |
				|  /   /  lower |
				1   /           |
				 1--------------2
			*/
			glm::vec3 upTri[3]{
				m_vertices[i][j],
				m_vertices[i + 1][j],
				m_vertices[i][j + 1]
			};
			glm::vec3 lowTri[3]{
				m_vertices[i][j + 1],
				m_vertices[i + 1][j],
				m_vertices[i + 1][j + 1]
			};

			m_faceNormals[0][i][j] = glm::normalize(glm::cross(upTri[0] - upTri[2], upTri[1] - upTri[2]));
			m_faceNormals[1][i][j] = glm::normalize(glm::cross(lowTri[0] - lowTri[2], lowTri[1] - lowTri[2]));
		}
	}
	// 2. sum up all per-face normals around a vertex to get per-vertex normals, i.e. smooth operation
	m_normals = std::vector<std::vector<glm::vec3>>(rows, std::vector<glm::vec3>(cols));
	for (GLuint i = 0; i < rows; i++) {
		for (GLuint j = 0; j < cols; j++) {
			glm::vec3 sum = glm::vec3(0.f);

			// top-left
			if (i != 0 && j != 0)
				sum += m_faceNormals[1][i - 1][j - 1];
			// top-right
			if (i != 0 && j != cols - 1) {
				sum += m_faceNormals[0][i - 1][j];
				sum += m_faceNormals[1][i - 1][j];
			}
			// bottom-right
			if (i != rows - 1 && j != cols - 1)
				sum += m_faceNormals[0][i][j];
			// bottom-left
			if (i != rows - 1 && j != 0) {
				sum += m_faceNormals[0][i][j - 1];
				sum += m_faceNormals[1][i][j - 1];
			}

			m_normals[i][j] = glm::normalize(sum);
		}
	}

}

void Terrain::bufferUpdate() {
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_EBO);

	glBindVertexArray(m_VAO);

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> texCoords;
	std::vector<glm::vec3> normals;

	for (GLuint i = 0; i < m_rows; i++) {
		for (GLuint j = 0; j < m_cols; j++) {
			vertices.push_back(m_vertices[i][j]);
			texCoords.push_back(m_texCoords[i][j]);
			normals.push_back(m_normals[i][j]);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, m_rows * m_cols * (2 * sizeof(glm::vec3) + sizeof(glm::vec2)), NULL, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(GLuint), &m_indices[0], GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(glm::vec3), &vertices[0]);
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.size() * sizeof(glm::vec2), &texCoords[0]);
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * (sizeof(glm::vec3) + sizeof(glm::vec2)), vertices.size() * sizeof(glm::vec3), &normals[0]);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(m_rows * m_cols * sizeof(glm::vec3)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(m_rows * m_cols * (sizeof(glm::vec3) + +sizeof(glm::vec2))));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

float Terrain::barryCentric(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec2 pos)
{
	GLfloat det = (p2.z - p3.z) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.z - p3.z);
	GLfloat l1 = ((p2.z - p3.z) * (pos.x - p3.x) + (p3.x - p2.x) * (pos.y - p3.z)) / det;
	GLfloat l2 = ((p3.z - p1.z) * (pos.x - p3.x) + (p1.x - p3.x) * (pos.y - p3.z)) / det;
	GLfloat l3 = 1.0f - l1 - l2;
	return l1 * p1.y + l2 * p2.y + l3 * p3.y;
}
