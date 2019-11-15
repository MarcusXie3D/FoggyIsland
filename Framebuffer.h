#pragma once

#include "texture2D.h"

#include <GL/glew.h>

#include <iostream>

/* Basic abstraction of OpenGL Framebuffer */
class Framebuffer
{
public:
	// State
	GLuint FBO, RBO;
	GLuint Width, Height;
	Texture2D ColorBuffer;
	Texture2D DepthBuffer;

	// Constructor
	Framebuffer(GLuint width, GLuint height) : Width(width), Height(height)
	{
		// Generate/configure framebuffer
		glGenFramebuffers(1, &FBO);
		// - Configure color buffer
		ColorBuffer.Internal_Format = GL_RGB16F; // Use floating point precision color attachment
		ColorBuffer.Filter_Min = GL_LINEAR;
		ColorBuffer.Mipmap = GL_FALSE;
		ColorBuffer.Generate(width, height, NULL);
		// - Configure depth buffer
		DepthBuffer.Image_Format = GL_DEPTH_COMPONENT;
		DepthBuffer.Internal_Format = GL_DEPTH_COMPONENT;
		DepthBuffer.Filter_Min = GL_LINEAR;
		DepthBuffer.Mipmap = GL_FALSE;
		DepthBuffer.Generate(width, height, NULL);
		// Attach to framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ColorBuffer.ID, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthBuffer.ID, 0);
		// Check if framebuffer is succesfully generated
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete; Check framebuffers!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// Update the texture used as the color buffer
	void UpdateColorBufferTexture(Texture2D& texture)
	{
		ColorBuffer = texture;
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ColorBuffer.ID, 0);
	}

	// Binds the current framebuffer (without clearing)
	void Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	}

	// Bind to framebuffer and clear buffers 
	void BeginRender()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		glClearColor(0.f, 0.f, 0.f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	// Unbind framebuffer to store data in buffers
	void EndRender()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
};
