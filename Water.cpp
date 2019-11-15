#include "Water.h"
#include "ResourceManager.h"
#include <glm\gtc\matrix_transform.hpp>
#include <iostream>

Water::Water(const glm::vec2 &size, const float &height) : m_size(size), m_height(height) {
	init_data();
	m_shader = ResourceManager::LoadShader("shaders/water.vs", "shaders/water.fs", nullptr, "Water_shader");
	glm::mat4 model;
	model = glm::translate(model, glm::vec3(0.f, height, 0.f));
	model = glm::scale(model, glm::vec3(size.x, 0.f, size.y));
	m_shader.SetMatrix4("model", model, GL_TRUE);

	// Texture samplers
	m_shader.Use();
	m_shader.SetInteger("refraction", 0);
	m_shader.SetInteger("reflection", 2);
}

Water::~Water() {
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_VBO);
}

void Water::load(std::string dudvMap, std::string normalMap, const float &scaleTex) {
	m_scaleTex = scaleTex;
	m_dudvMap = ResourceManager::LoadTexture(dudvMap.c_str(), GL_FALSE, "Water_dudvMap", GL_FALSE);
	m_normalMap = ResourceManager::LoadTexture(normalMap.c_str(), GL_FALSE, "Water_normalMap", GL_FALSE);
	// Texture samplers
	m_shader.Use();
	m_shader.SetInteger("dudvMap", 1);
	m_shader.SetInteger("normalMap", 3);
	m_shader.SetFloat("scaleTex", scaleTex);
}

void Water::render() {
	// Textures
	glActiveTexture(GL_TEXTURE0);
	m_texRefraction.Bind();
	glActiveTexture(GL_TEXTURE1);
	m_dudvMap.Bind();
	glActiveTexture(GL_TEXTURE2);
	m_texReflection.Bind();
	glActiveTexture(GL_TEXTURE3);
	m_normalMap.Bind();
	// Render
	glBindVertexArray(m_VAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void Water::initPassRefraction() {
	glEnable(GL_CLIP_DISTANCE0);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glClearColor(0.18f, 0.2f, 0.18f, 1.0f);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texRefraction.ID, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Water::terminatePassRefraction() {
	glDisable(GL_CLIP_DISTANCE0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Water::initPassReflection() {
	glFrontFace(GL_CW);// in reflection pass, camera is mirrored, so faces are mirrored too
	glEnable(GL_CLIP_DISTANCE0);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glClearColor(0.18f, 0.2f, 0.18f, 1.0f);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texReflection.ID, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Water::terminatePassReflection() {
	glFrontFace(GL_CCW);// in reflection pass, camera is mirrored, so faces are mirrored too
	glDisable(GL_CLIP_DISTANCE0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Water::init_data() {
	// Initialize water quad
	float vertexData[] = {
		// Position         // Texture
		-0.5f, 0.0f, -0.5f, 0.0f, 0.0f,
		-0.5f, 0.0f,  0.5f, 0.0f, 1.0f,
		 0.5f, 0.0f, -0.5f, 1.0f, 0.0f,
		 0.5f, 0.0f,  0.5f, 1.0f, 1.0f
	};

	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Initialize Textures (for FBO colorbuffers)
	m_texRefraction.Mipmap = false;
	m_texRefraction.Internal_Format = GL_RGB;
	m_texRefraction.Filter_Min = GL_LINEAR;
	m_texRefraction.Generate(1280, 720, NULL);
	m_texReflection.Mipmap = false;
	m_texReflection.Internal_Format = GL_RGB;
	m_texReflection.Filter_Min = GL_LINEAR;
	m_texReflection.Generate(1280, 720, NULL);

	// generate framebuffer
	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glGenRenderbuffers(1, &m_RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1280, 720);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RBO);

	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR:FRAMEBUFFER: WATER!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
