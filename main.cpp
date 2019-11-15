#define GLEW_STATIC

#include "Water.h"
#include "Skybox.h"
#include "ResourceManager.h"
#include "camera.h"
#include "Terrain.h"
#include "Model.h"
#include "Light.h"
#include "Fog.h"
#include "Framebuffer.h"
#include "Geometry.h"

#include <gl/glew.h>
#include <iostream>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>

Camera camera(glm::vec3(0.0f, 10.0f, 0.0f));

// settings
const GLuint SCR_WIDTH = 1280;
const GLuint SCR_HEIGHT = 720;
const float near = 0.1f;
const float far = 750.0f;
float heightScale = 0.1f;
glm::vec2 terrainWaterSize = glm::vec2(750.0f);
float waterHeight = 2.f;
const GLuint NR_TREES = 250; 
const GLfloat TREE_SCALE = 2.0f;
const GLfloat HOUSE_SCALE = 0.7f;

//camera data for generating view matrix
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
float pitch{ 0.0f }, yaw{ -90.0f };
float fov{ 45.0f };

float deltaTime{ 0.0f };
float UpdateTime{ 0.0f };

// Light
const glm::vec3 lightDir = glm::normalize(glm::vec3(0.43f, 0.76f, -0.33f));//(0.6f, 0.76f, -0.25f)
const glm::vec3 lightColor = glm::vec3(1.f, 1.f, 0.7f);
const GLfloat ambientStrength = 0.08;
const glm::vec3 ambientColor = glm::vec3(0.25f, 0.25f, 1.f);

// Fog
const float fogDensity = 0.0045f;
const glm::vec3 fogColor1 = glm::vec3(0.604f, 0.655f, 0.718f);
const glm::vec3 fogColor2 = glm::vec3(0.631f, 0.651f, 0.698f);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

bool cursorFlag{ false };

int main()
{
	//Initiallize GLFW and generate window
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Foggy Island", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Initialize GLEW to setup the OpenGL Function pointers
	glewExperimental = GL_TRUE;
	glewInit();
	glGetError();

	// Set Window
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Display waiting information while program is loading up
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	Shader loadingShader = ResourceManager::LoadShader("shaders/post_processing.vs", "shaders/loading.fs", nullptr, "quad");
	Texture2D loadingTexture = ResourceManager::LoadTexture("resources/textures/LoadingPicture.png", GL_TRUE, "lensstar");
	loadingShader.SetInteger("loadingPicture", 0, GL_TRUE);
	loadingTexture.Bind(0);
	Geometry::DrawPlane();
	glfwSwapBuffers(window);

	// Load Models
	Model house("models/house/farmhouse.obj");
	house.calculateBoundingVolume();
	Model tree("models/tree3/laubbaum.obj"); 
	tree.calculateBoundingVolume();

	// Load Shaders
	Shader shaderSkybox = ResourceManager::LoadShader("shaders/skybox.vs", "shaders/skybox.fs", nullptr, "shaderSkybox");
	Shader shaderTerrain = ResourceManager::LoadShader("shaders/terrain.vs", "shaders/terrain.fs", nullptr, "shaderTerrain");
	Shader shaderHouse = ResourceManager::LoadShader("shaders/house.vs", "shaders/house.fs", nullptr, "shaderHouse");
	Shader SimpleShader = ResourceManager::LoadShader("shaders/simple.vs", "shaders/simple.fs", nullptr, "SimpleShader");
	Shader treeShader = ResourceManager::LoadShader("shaders/tree.vs", "shaders/tree.fs", nullptr, "treeShader");
	Shader treeSimpleShader = ResourceManager::LoadShader("shaders/simple_tree.vs", "shaders/simple_tree.fs", nullptr, "treeSimpleShader");
	Shader sunShader = ResourceManager::LoadShader("shaders/sun.vs", "shaders/sun.fs", nullptr, "quad");
	Shader volumetricShader = ResourceManager::LoadShader("shaders/post_processing.vs", "shaders/volumetric_lighting.fs", nullptr, "quad");
	Shader gaussianBlurShader = ResourceManager::LoadShader("shaders/post_processing.vs", "shaders/gaussian_blur.fs", nullptr, "quad");
	Shader applyPostProcessShader = ResourceManager::LoadShader("shaders/post_processing.vs", "shaders/applyPostProcess.fs", nullptr, "quad");

	// Load Textures
	Texture2D textureTerrain = ResourceManager::LoadTexture("resources/textures/grass_COLOR.png", GL_FALSE, "textureTerrain");
	Texture2D SunTexture = ResourceManager::LoadTexture("resources/textures/sun.png", GL_TRUE, "lensstar");

	// Configure Texture Samplers
	shaderTerrain.SetInteger("terrain", 0, GL_TRUE);
	shaderTerrain.SetInteger("shadowMap", 1);
	shaderHouse.SetInteger("material.texture_diffuse1", 0, GL_TRUE);
	shaderHouse.SetInteger("material.texture_specular1", 1);
	shaderHouse.SetInteger("material.texture_normal1", 2);
	shaderHouse.SetInteger("lightDepthTexture", 3);
	shaderHouse.SetFloat("material.shininess", 16.0f);
	treeShader.SetInteger("texturez", 0, GL_TRUE);
	treeShader.SetInteger("shadowMap", 3);
	volumetricShader.SetInteger("scene", 0, GL_TRUE);
	gaussianBlurShader.SetInteger("image", 0, GL_TRUE);
	applyPostProcessShader.SetInteger("scene", 0, GL_TRUE);
	applyPostProcessShader.SetInteger("normalScene", 1);

	// Terrain
	Terrain terrain;
	terrain.load(terrainWaterSize, 37.5f, 300.0f, "resources/textures/heightmap_island_low_poly.jpg");
	camera.loadTerrain(&terrain);

	// Water
	Water water(terrainWaterSize, waterHeight);
	water.load("resources/textures/water_dudv_blur.jpg", "resources/textures/water_normal.jpg", 100.f);

	// Trees
	const GLuint NR_TREES = 250; const GLfloat TREE_SCALE = 2.0f;
	srand(2348);
	std::vector<glm::mat4> trees;
	for (GLuint i = 0; i < NR_TREES; i++) {
		GLint x = rand() % (int)terrain.getSize().x - terrain.getSize().x * 0.5f;
		GLint z = rand() % (int)terrain.getSize().y - terrain.getSize().y * 0.5f;
		float y = terrain.getHeight(x, z);
		if (y < water.getHeight() + 0.5f)
			continue;
		glm::mat4 model;
		GLfloat scale = TREE_SCALE + ((rand() % 25) - 7.5) / 10.0f;
		model = glm::translate(model, glm::vec3(x, y - 0.05 * scale, z));
		model = glm::scale(model, glm::vec3(scale));
		trees.push_back(model);
	}

	// Houses
	std::vector<glm::vec3> houseLocs;
	houseLocs.push_back(glm::vec3(-200, terrain.getHeight(-200, 80), 80));
	houseLocs.push_back(glm::vec3(-225, terrain.getHeight(-225, 50), 50));
	houseLocs.push_back(glm::vec3(-226, terrain.getHeight(-226, 135), 135));
	houseLocs.push_back(glm::vec3(-23, terrain.getHeight(-23, 172), 172));
	houseLocs.push_back(glm::vec3(260.0f, terrain.getHeight(260.0f, 15.0f), 15.0f));
	std::vector<glm::mat4> housesModels;
	for (GLuint i = 0; i < houseLocs.size(); i++) {
		glm::mat4 model;
		model = glm::translate(model, houseLocs[i]);
		model = glm::scale(model, glm::vec3(HOUSE_SCALE));
		housesModels.push_back(model);
	}
	
	// Skybox
	Skybox skybox(&shaderSkybox);

	skybox.loadDiurnalSkybox(
		"resources/skybox/right.png",
		"resources/skybox/left.png",
		"resources/skybox/top.png",
		"resources/skybox/bottom.png",
		"resources/skybox/back.png",
		"resources/skybox/front.png"
	);

	skybox.loadNocturnalSkybox(
		"resources/skybox/night/right.png",
		"resources/skybox/night/left.png",
		"resources/skybox/night/top.png",
		"resources/skybox/night/bottom.png",
		"resources/skybox/night/back.png",
		"resources/skybox/night/front.png"
	);

	// Shadow framebuffer
	GLuint const SHADOW_RESOLUTION = 4096; //8192;//
	GLuint ShadowFBO;
	Texture2D shadowDepth;
	shadowDepth.Image_Format = GL_DEPTH_COMPONENT;
	shadowDepth.Internal_Format = GL_DEPTH_COMPONENT;
	shadowDepth.Filter_Min = GL_LINEAR;
	// Manually set depth attachment to clamp to border value of 1.0 depth; ensures areas that are not visible from light are not in shadow.
	shadowDepth.Wrap_S = GL_CLAMP_TO_BORDER;
	shadowDepth.Wrap_T = GL_CLAMP_TO_BORDER;
	shadowDepth.Generate(SHADOW_RESOLUTION, SHADOW_RESOLUTION, NULL);
	shadowDepth.Bind();
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glGenFramebuffers(1, &ShadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ShadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepth.ID, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::SHADOW_FRAMEBUFFER" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	float lastTime{ 0.0f };

	// Set Projection Matrix
	glm::mat4 projection = camera.SetProjectionMatrix((float)SCR_WIDTH, (float)SCR_HEIGHT, near, far); // this remains unchanged for every frame
	shaderTerrain.SetMatrix4("projection", projection, GL_TRUE);
	water.m_shader.SetMatrix4("projection", projection, GL_TRUE);
	shaderHouse.SetMatrix4("projection", projection, GL_TRUE);
	treeShader.SetMatrix4("projection", projection, GL_TRUE);
	sunShader.SetMatrix4("projection", projection, GL_TRUE);

	// Sun
	Light sun(lightDir, lightColor, ambientStrength, ambientColor);
	sun.setShader(shaderHouse, "sun", GL_TRUE);
	sun.setShader(shaderTerrain, "sun", GL_TRUE);
	sun.setShader(water.m_shader, "sun", GL_TRUE);
	sun.setShader(treeShader, "sun", GL_TRUE);

	// Fog
	Fog fog(fogDensity, fogColor1);
	fog.setShader(shaderTerrain, "fog", GL_TRUE);
	fog.setShader(water.m_shader, "fog", GL_TRUE);
	fog.setShader(shaderHouse, "fog", GL_TRUE);
	fog.setShader(treeShader, "fog", GL_TRUE);
	fog.setShader(shaderSkybox, "fog", GL_TRUE);

	// Trees - Instanced array
	GLuint VBO_Trees;
	glGenBuffers(1, &VBO_Trees);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Trees);
	glBufferData(GL_ARRAY_BUFFER, trees.size() * sizeof(glm::mat4), NULL, GL_STREAM_DRAW);
	for (GLuint i = 0; i < tree.Meshes.size(); i++){
		glBindVertexArray(tree.Meshes[i].VAO);

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (GLvoid*)0);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (GLvoid*)(sizeof(glm::vec4)));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (GLvoid*)(2 * sizeof(glm::vec4)));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (GLvoid*)(3 * sizeof(glm::vec4)));

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);
	}

	// Render to Texture
	Framebuffer intermediateFramebuffer(SCR_WIDTH, SCR_HEIGHT);
	Framebuffer normalFramebuffer(SCR_WIDTH, SCR_HEIGHT);
	Framebuffer volumetricFBO(SCR_WIDTH, SCR_HEIGHT);
	Texture2D buffer1, buffer2; // Buffers used for ping-ponging between color attachments for Gaussian blur
	buffer1.Internal_Format = GL_RGB16F;
	buffer1.Filter_Min = GL_LINEAR; buffer1.Mipmap = GL_FALSE;
	buffer1.Wrap_S = GL_CLAMP_TO_EDGE; buffer1.Wrap_T = GL_CLAMP_TO_EDGE;
	buffer1.Generate(SCR_WIDTH, SCR_HEIGHT, NULL);

	buffer2.Internal_Format = GL_RGB16F;
	buffer2.Filter_Min = GL_LINEAR; buffer2.Mipmap = GL_FALSE;
	buffer2.Wrap_S = GL_CLAMP_TO_EDGE; buffer2.Wrap_T = GL_CLAMP_TO_EDGE;
	buffer2.Generate(SCR_WIDTH, SCR_HEIGHT, NULL);

	volumetricFBO.ColorBuffer.Bind();
	volumetricFBO.ColorBuffer.Wrap_S = GL_CLAMP_TO_EDGE; // Clamp to edge so values do not leak into other sides of texture
	volumetricFBO.ColorBuffer.Wrap_T = GL_CLAMP_TO_EDGE;

	//Render loop
	while (!glfwWindowShouldClose(window)){
		float currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;
		UpdateTime += deltaTime;

		if (UpdateTime > 1.f) {
			UpdateTime = 0.f;
			glfwSetWindowTitle(window, ("Foggy Island   FPS : " + std::to_string(1.f / deltaTime)).c_str());
		}

		processInput(window);

		// configure view matrices
		glm::mat4 view = camera.GetViewMatrix();
		camera.CalculateViewFrustum();
		glm::mat4 matProjectionView = projection * view;

		// cull trees that are out of frustum
		std::vector<glm::mat4> treeModels;
		for (GLuint i = 0; i < trees.size(); i++) {
			if (tree.isInFrustum(camera, trees[i]))
				treeModels.push_back(trees[i]);
		}
		if (treeModels.size() > 0) {
			glBindBuffer(GL_ARRAY_BUFFER, VBO_Trees);
			glBufferSubData(GL_ARRAY_BUFFER, 0, treeModels.size() * sizeof(glm::mat4), &treeModels[0]);
		}

#pragma region SHADOW
		/////////////////////////////////////////////////////////
		///////////////////  SHADOW PASS  ///////////////////////
		/////////////////////////////////////////////////////////

		glViewport(0, 0, SHADOW_RESOLUTION, SHADOW_RESOLUTION);
		glBindFramebuffer(GL_FRAMEBUFFER, ShadowFBO);
		glClear(GL_DEPTH_BUFFER_BIT);

		glm::mat4 lightMatrix; GLfloat orthoWidth = 150.0f;
		glm::mat4 lightProjection = glm::ortho(-orthoWidth, orthoWidth, -orthoWidth, orthoWidth, 25.0f, 350.0f);
		glm::mat4 lightView = glm::lookAt(sun.m_direction * 250.f + glm::vec3(camera.Position.x, 0.0f, camera.Position.z), glm::vec3(0.0f) + glm::vec3(camera.Position.x, 0.0f, camera.Position.z), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 biasMatrix = glm::mat4();
		biasMatrix[0][0] = 0.5; biasMatrix[0][1] = 0.0; biasMatrix[0][2] = 0.0; biasMatrix[0][3] = 0.0;
		biasMatrix[1][0] = 0.0; biasMatrix[1][1] = 0.5; biasMatrix[1][2] = 0.0; biasMatrix[1][3] = 0.0;
		biasMatrix[2][0] = 0.0; biasMatrix[2][1] = 0.0; biasMatrix[2][2] = 0.5; biasMatrix[2][3] = 0.0;
		biasMatrix[3][0] = 0.5; biasMatrix[3][1] = 0.5; biasMatrix[3][2] = 0.5; biasMatrix[3][3] = 1.0;
		lightMatrix = lightProjection * lightView; // Render texture as full texture, add bias only to final matrix (to reposition vertex to 0.0 - 1.0f space)

		/***********************Terrain*********************/
		SimpleShader.Use();
		SimpleShader.SetMatrix4("lightMatrix", lightMatrix);
		SimpleShader.SetMatrix4("model", glm::mat4());
		terrain.render();

		/***********************Houses*********************/
		for (GLuint i = 0; i < housesModels.size(); i++) {
			SimpleShader.SetMatrix4("model", housesModels[i]);
			house.Draw(SimpleShader);
		}
		
		/***********************Trees*********************/
		if (treeModels.size() > 0) {
			glDisable(GL_CULL_FACE);
			treeSimpleShader.Use();
			treeSimpleShader.SetMatrix4("lightMatrix", lightMatrix);
			treeSimpleShader.SetFloat("time", glfwGetTime());
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tree.Meshes[0].textures[0].id);
			for (GLuint i = 0; i < tree.Meshes.size(); i++) {
				glBindVertexArray(tree.Meshes[i].VAO);
				glDrawElementsInstanced(GL_TRIANGLES, tree.Meshes[i].indices.size(), GL_UNSIGNED_INT, 0, treeModels.size());
			}
			glEnable(GL_CULL_FACE);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		lightMatrix = biasMatrix * lightMatrix; // Add bias to lightMatrix (convert NDC to [0.0, 1.0] interval)

#pragma endregion SHADOW
		
#pragma region REFRACTION
		///////////////////////////////////////////////////////////
		///////////////////  REFRACTION PASS  /////////////////////
		///////////////////////////////////////////////////////////

		water.initPassRefraction();

		/**********************Terrain********************/
		shaderTerrain.SetMatrix4("view", view, GL_TRUE);
		shaderTerrain.SetInteger("isRefraction", GL_TRUE);
		shaderTerrain.SetInteger("isReflection", GL_FALSE);
		shaderTerrain.SetFloat("waterHeight", water.getHeight());
		shaderTerrain.SetMatrix4("shadowMat", lightMatrix);
		shaderTerrain.SetVector3f("viewPos", camera.Position);
		textureTerrain.Bind(0);
		shadowDepth.Bind(1);
		terrain.render();

		water.terminatePassRefraction();

#pragma endregion REFRACTION
		
#pragma region REFLECTION
		///////////////////////////////////////////////////////////
		///////////////////  REFLECTION PASS  /////////////////////
		///////////////////////////////////////////////////////////

		// now use a imaginary camera on the counter position under watersurface
		glm::mat4 imgView = camera.GetImaginaryViewMatrix(water.getHeight());

		water.initPassReflection();
		
		/**********************Terrain********************/
		shaderTerrain.SetMatrix4("view", imgView, GL_TRUE);
		shaderTerrain.SetInteger("isRefraction", GL_FALSE);
		shaderTerrain.SetInteger("isReflection", GL_TRUE);
		shaderTerrain.SetFloat("waterHeight", water.getHeight());
		shaderTerrain.SetMatrix4("shadowMat", lightMatrix);
		shaderTerrain.SetVector3f("viewPos", camera.Position);
		textureTerrain.Bind(0);
		shadowDepth.Bind(1);
		terrain.render();
		
		/**********************Trees********************/
		if (treeModels.size() > 0)
		{
			glDisable(GL_CULL_FACE);
			treeShader.SetMatrix4("view", imgView, GL_TRUE);
			treeShader.SetVector3f("viewPos", camera.Position);
			treeShader.SetFloat("time", glfwGetTime());
			treeShader.SetMatrix4("shadowMat", lightMatrix);
			shadowDepth.Bind(3);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tree.Meshes[0].textures[0].id);
			for (GLuint i = 0; i < tree.Meshes.size(); ++i)
			{
				glBindVertexArray(tree.Meshes[i].VAO);
				glDrawElementsInstanced(GL_TRIANGLES, tree.Meshes[i].indices.size(), GL_UNSIGNED_INT, 0, treeModels.size());
			}
			glEnable(GL_CULL_FACE);
		}

		/***********************Skybox*********************/
		skybox.render(imgView, projection, 1.f);

		water.terminatePassReflection();

#pragma endregion REFLECTION

		// Check if the sun is in our sight. If not, skip GOD RAYS pass and POST PROCESSING pass.
		glm::vec4 position = projection * glm::mat4(glm::mat3(view)) * glm::vec4(sun.m_direction * 250.f, 1.f);
		float zValue = position.z;
		glm::vec2 sunPos = glm::vec2(position.x, position.y);
		sunPos /= position.w;
		sunPos = (sunPos * 0.5f) + 0.5f;
		bool doGodRays = true;
		const float margin = 0.05f;
		if (zValue < 0.f || sunPos.x < 0.f - margin || sunPos.x > 1.f + margin || sunPos.y < 0.f - margin || sunPos.y > 1.f + margin)//the interval should be slightly larger than [0, 1], otherwise when the sun appear from the edges of the screen it looks ∫‹Õª»ª
			doGodRays = false;

#pragma region GOD_RAYS
		///////////////////////////////////////////////////////////
		///////////////////////  GOD RAYS  ////////////////////////
		///////////////////////////////////////////////////////////
		if (doGodRays) 
		{
			intermediateFramebuffer.BeginRender();

			SimpleShader.Use();
			SimpleShader.SetMatrix4("lightMatrix", matProjectionView);

			/***********************Houses*********************/
			for (GLuint i = 0; i < housesModels.size(); i++)
			{
				if (house.isInFrustum(camera, housesModels[i]))
				{
					SimpleShader.SetMatrix4("model", housesModels[i]);
					house.Draw(SimpleShader);
				}
			}

			/**********************Terrain********************/
			SimpleShader.SetMatrix4("model", glm::mat4());
			terrain.render();

			/***********************Water*********************/
			glm::mat4 model;
			model = glm::translate(model, glm::vec3(0.0f, water.getHeight(), 0.0f));
			model = glm::scale(model, glm::vec3(water.getSize().x, 1.0f, water.getSize().y));
			SimpleShader.SetMatrix4("model", model);
			glBindVertexArray(water.m_VAO);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			/**********************Trees********************/
			if (treeModels.size() > 0)
			{
				glDisable(GL_CULL_FACE);
				treeSimpleShader.Use();
				treeSimpleShader.SetMatrix4("lightMatrix", matProjectionView);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, tree.Meshes[0].textures[0].id);
				for (GLuint i = 0; i < tree.Meshes.size(); ++i)
				{
					glBindVertexArray(tree.Meshes[i].VAO);
					glDrawElementsInstanced(GL_TRIANGLES, tree.Meshes[i].indices.size(), GL_UNSIGNED_INT, 0, treeModels.size());
				}
				glEnable(GL_CULL_FACE);
			}

			/***********************Sun*********************/
			sunShader.Use();
			sunShader.SetMatrix4("view", glm::mat4(glm::mat3(view)));// when player walks forward the sun won't be left behind, so to keep the sun around the player, don't translate
			sunShader.SetVector3f("sunPos", sun.m_direction * 250.f);// check here when the sun quad is too big // debug
			SunTexture.Bind(0);
			Geometry::DrawPlane();

			intermediateFramebuffer.EndRender();
		}
#pragma endregion GOD_RAYS

#pragma region NORMAL
		///////////////////////////////////////////////////////////
		/////////////////////  NORMAL PASS  ///////////////////////
		///////////////////////////////////////////////////////////

		if(doGodRays)
			normalFramebuffer.BeginRender();
		else {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		/**********************Terrain********************/
		shaderTerrain.SetMatrix4("view", view, GL_TRUE);
		shaderTerrain.SetInteger("isRefraction", GL_FALSE);
		shaderTerrain.SetInteger("isReflection", GL_FALSE);
		shaderTerrain.SetMatrix4("shadowMat", lightMatrix);
		shaderTerrain.SetVector3f("viewPos", camera.Position);
		textureTerrain.Bind(0);
		shadowDepth.Bind(1);
		terrain.render();

		/***********************Water*********************/
		water.m_shader.SetMatrix4("view", view, GL_TRUE);
		water.m_shader.SetFloat("time", glfwGetTime() / 10.f);
		water.m_shader.SetVector3f("viewPos", camera.Position);
		water.render();

		/***********************Houses*********************/
		shaderHouse.SetMatrix4("view", view, GL_TRUE);
		shaderHouse.SetVector3f("viewPos", camera.Position);
		for (GLuint i = 0; i < housesModels.size(); i++) {
			if (house.isInFrustum(camera, housesModels[i])) {
				shaderHouse.SetMatrix4("model", housesModels[i]);
				house.Draw(shaderHouse);
			}
		}

		/**********************Trees********************/
		if (treeModels.size() > 0)
		{
			glDisable(GL_CULL_FACE);
			treeShader.SetMatrix4("view", view, GL_TRUE);
			treeShader.SetVector3f("viewPos", camera.Position);
			treeShader.SetFloat("time", glfwGetTime());
			treeShader.SetMatrix4("shadowMat", lightMatrix);
			shadowDepth.Bind(3);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tree.Meshes[0].textures[0].id);
			for (GLuint i = 0; i < tree.Meshes.size(); ++i)
			{
				glBindVertexArray(tree.Meshes[i].VAO);
				glDrawElementsInstanced(GL_TRIANGLES, tree.Meshes[i].indices.size(), GL_UNSIGNED_INT, 0, treeModels.size());
			}
			glEnable(GL_CULL_FACE);
		}

		/***********************Skybox*********************/
		skybox.render(view, projection, 1.f);

		if(doGodRays)
			normalFramebuffer.EndRender();
#pragma endregion NORMAL

#pragma region POST_PROCESSING
///////////////////////////////////////////////////////////
////////////////////  POST PROCESSING  ////////////////////
///////////////////////////////////////////////////////////
		if (doGodRays) {
			glDisable(GL_DEPTH_TEST);

			volumetricFBO.BeginRender();

			volumetricShader.SetVector2f("sunPos", sunPos, true);

			volumetricShader.Use();
			intermediateFramebuffer.ColorBuffer.Bind(0);
			Geometry::DrawPlane();

			volumetricFBO.EndRender();

			// Gaussian blur
			int blur_iterations = 4;
			buffer1 = volumetricFBO.ColorBuffer;
			volumetricFBO.Bind();
			volumetricFBO.UpdateColorBufferTexture(buffer2);
			gaussianBlurShader.Use();
			volumetricFBO.BeginRender();
			for (int i = 0; i < blur_iterations; i++) {
				for (int n = 0; n < 2; n++) {
					// Horizontal or vertical based on n
					gaussianBlurShader.SetInteger("horizontal", n == 0 ? 1 : 0);
					volumetricFBO.UpdateColorBufferTexture(n == 0 ? buffer2 : buffer1);
					if (n == 0)
						buffer1.Bind(0);
					else
						buffer2.Bind(0);
					Geometry::DrawPlane();
				}
			}
			volumetricFBO.EndRender();

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			applyPostProcessShader.Use();
			volumetricFBO.ColorBuffer.Bind(0);
			normalFramebuffer.ColorBuffer.Bind(1);
			Geometry::DrawPlane();

			glEnable(GL_DEPTH_TEST);
		}

#pragma endregion POST_PROCESSING

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ResourceManager::Clear();
	glfwTerminate();

	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	//move camera
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		cursorFlag = !cursorFlag;
		if(cursorFlag)
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		else
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float offsetX = xpos - lastX;
	float offsetY = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(offsetX, offsetY);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}
