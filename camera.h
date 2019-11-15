#ifndef CAMERA_H
#define CAMERA_H

//#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

#include "Terrain.h"

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 30.f;//8.f;
const float SENSITIVITY = 0.03f;//0.1f;
const float ZOOM = 45.0f;

struct Plane {
	glm::vec3 Normal;
	glm::vec3 Point;
	GLfloat   D;

	void CalcDistance() {
		D = glm::dot(-Normal, Point); // See http://www.lighthouse3d.com/tutorials/maths/plane/
	}

	GLfloat Distance(glm::vec3 point) {
		return glm::dot(Normal, point) + D;
	}
};


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
	// Camera Attributes
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	// Euler Angles
	float Yaw;
	float Pitch;
	// Camera options
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;// in degrees
	// View frustum
	Plane Frustum[6]; // Order: Near, Far, Top, Bottom, Left, Right
	GLfloat Near, Far;
	GLfloat NearWidth, NearHeight; // Width and height of near plane
	GLfloat FarWidth, FarHeight; // Width and height of far plane

	// Constructor with vectors
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(-0.826095f, -0.00348972f, -0.563521f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}
	// Constructor with scalar values
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = glm::vec3(posX, posY, posZ);
		WorldUp = glm::vec3(upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	void loadTerrain(Terrain *terrain) {
		m_terrain = terrain;
	}

	// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(Position, Position + Front, Up);
	}

	// Used for water reflection. An imaginary camera on the symmetric positon, which is under the water surface.
	glm::mat4 GetImaginaryViewMatrix(const float &waterHeight)
	{
		glm::vec3 imgPos = glm::vec3(Position.x, 2.f * waterHeight - Position.y, Position.z);
		glm::vec3 imgFront = glm::vec3(Front.x, -Front.y, Front.z);
		glm::vec3 imgUp = glm::vec3(Up.x, -Up.y, Up.z);
		return glm::scale(glm::mat4(), glm::vec3(-1.0f, 1.0f, 1.0f)) * glm::lookAt(imgPos, imgPos + imgFront, imgUp);
	}

	// Sets the currently active projection matrix of the camera
	glm::mat4 SetProjectionMatrix(GLfloat width, GLfloat height, GLfloat near, GLfloat far)
	{
		// Calculate Aspect, and widths and height of near/far plane (required for frustum calculations)
		GLfloat ratio = width / height;
		NearHeight = 2 * tan(glm::radians(this->Zoom) / 2.0) * near;
		NearWidth = NearHeight * ratio;
		FarHeight = 2 * tan(glm::radians(this->Zoom) / 2.0) * far;
		FarWidth = FarHeight * ratio;
		// Finally store projection matrix
		this->Near = near;
		this->Far = far;
		this->Projection = glm::perspective(glm::radians(this->Zoom), ratio, near, far);
		return this->Projection;
	}

	// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(Camera_Movement direction, float deltaTime)
	{
		float velocity = MovementSpeed * deltaTime;
		if (direction == FORWARD)
			Position += Front * velocity;
		if (direction == BACKWARD)
			Position -= Front * velocity;
		if (direction == LEFT)
			Position -= Right * velocity;
		if (direction == RIGHT)
			Position += Right * velocity;
		
		float terrainHeight = m_terrain->getHeight(Position.x, Position.z);
		if (Position.y < terrainHeight + 6.f)
			Position.y = terrainHeight + 6.f;
		//std::cout << "Position = " << Position.x << ", " << Position.y << ", " << Position.z << "\n";
	}

	// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}

		// Update Front, Right and Up Vectors using the updated Euler angles
		updateCameraVectors();
	}

	// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void ProcessMouseScroll(float yoffset)
	{
		if (Zoom >= 1.0f && Zoom <= 45.0f)
			Zoom -= yoffset;
		if (Zoom <= 1.0f)
			Zoom = 1.0f;
		if (Zoom >= 45.0f)
			Zoom = 45.0f;
	}

	// (Re)calculates the view frustum planes of the camera's current position/orientation.
	// For a more in-depth description of the algorithm see http://www.lighthouse3d.com/tutorials/view-frustum-culling/geometric-approach-extracting-the-planes/
	void CalculateViewFrustum()
	{
		// 1. First calculate all 8 points of the frustum (corners)
		// - Naming convention: (n=near,f=far)(t=top,b=bottom)(l=left,r=right)
		// - - Near
		glm::vec3 nc = this->Position + this->Front * this->Near; // nc = near-center

		glm::vec3 ntl = nc + (this->Up * this->NearHeight / 2.0f) - (this->Right * this->NearWidth / 2.0f);
		glm::vec3 ntr = nc + (this->Up * this->NearHeight / 2.0f) + (this->Right * this->NearWidth / 2.0f);
		glm::vec3 nbl = nc - (this->Up * this->NearHeight / 2.0f) - (this->Right * this->NearWidth / 2.0f);
		glm::vec3 nbr = nc - (this->Up * this->NearHeight / 2.0f) + (this->Right * this->NearWidth / 2.0f);
		// - - Far
		glm::vec3 fc = this->Position + this->Front * this->Far; // fc = far-center

		glm::vec3 ftl = fc + (this->Up * this->FarHeight / 2.0f) - (this->Right * this->FarWidth / 2.0f);
		glm::vec3 ftr = fc + (this->Up * this->FarHeight / 2.0f) + (this->Right * this->FarWidth / 2.0f);
		glm::vec3 fbl = fc - (this->Up * this->FarHeight / 2.0f) - (this->Right * this->FarWidth / 2.0f);
		glm::vec3 fbr = fc - (this->Up * this->FarHeight / 2.0f) + (this->Right * this->FarWidth / 2.0f);

		// 2. Create planes out of plane normals and positions (note: all normals point inside frustum)
		// - 2.1 - Near/Far plane (trick: use front vector as normal and nc/fc as points on pane)
		this->Frustum[0].Normal = this->Front;
		this->Frustum[0].Point = nc;
		this->Frustum[1].Normal = -this->Front;
		this->Frustum[1].Point = fc;
		// - 2.2 - Top/Bottom planes (cross vector with up and vector between top/bottom side of near/far plane and position
		glm::vec3 difference = (nc + this->Up * this->NearHeight / 2.0f) - this->Position;
		this->Frustum[2].Normal = glm::cross(glm::normalize(difference), this->Right);
		this->Frustum[2].Point = this->Position; // Position is on the plane due to perspective projection's pyramid form
		difference = (nc - this->Up * this->NearHeight / 2.0f) - this->Position;
		this->Frustum[3].Normal = glm::cross(this->Right, glm::normalize(difference));
		this->Frustum[3].Point = this->Position;
		// - 2.3 - Left/Right planes (cross vector with up and vector between left/right side of near/far plane and position
		difference = (nc - this->Right * this->NearWidth / 2.0f) - this->Position;
		this->Frustum[4].Normal = glm::cross(glm::normalize(difference), this->Up);
		this->Frustum[4].Point = this->Position;
		difference = (nc + this->Right * this->NearWidth / 2.0f) - this->Position;
		this->Frustum[5].Normal = glm::cross(this->Up, glm::normalize(difference));
		this->Frustum[5].Point = this->Position;

		// 3. Calculate D (distance) value of plane equation for each of the frustum planes
		for (GLuint i = 0; i < 6; ++i)
			this->Frustum[i].CalcDistance();
	}

private:
	Terrain *m_terrain;

	// Transformations
	glm::mat4 Projection;

	// Calculates the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors()
	{
		// Calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);
		//Front = glm::normalize(glm::vec3(-0.801772f, 0.f, -0.597629f));// debug
		// Also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));
		//std::cout << "Front = " << Front.x << ", " << Front.y << ", " << Front.z << "\n";
	}
};
#endif