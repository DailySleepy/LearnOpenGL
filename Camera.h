#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <iostream>

using namespace glm;

enum Direction
{
	FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float CAMERA_SPEED = 10.0f;
const float SENSITIVITY = 0.05f;
const float FOV = 45.0f;

class Camera
{
public:
	vec3 position;
	vec3 front;
	vec3 right;
	vec3 up;
	vec3 worldUp;
	float yaw, pitch;
	float speed;
	float sensitivity;
	float fov;
	float aspectRatio;
	float zNear, zFar;

	bool mouseMovementState = false;

	Camera(vec3 position = vec3(0, 0, 0), vec3 worldUp = vec3(0, 1, 0), float yaw = YAW, float pitch = PITCH)
		: front(0, 0, -1), speed(CAMERA_SPEED), sensitivity(SENSITIVITY), fov(FOV)
	{
		this->position = position;
		this->worldUp = worldUp;
		this->yaw = yaw;
		this->pitch = pitch;
		updateCameraVector();
	}

	void setAspectRatio(float aspectRatio)
	{
		this->aspectRatio = aspectRatio;
	}

	mat4 getViewMatrix()
	{
		return lookAt(position, position + front, up);
	}

	mat4 getProjectionMatrix(float zNear = 0.1f, float zFar = 100.0f)
	{
		this->zNear = zNear; this->zFar = zFar;
		return perspective(radians(fov), aspectRatio, zNear, zFar);
	}
	mat4 getProjectionMatrix(float left, float right, float bottom, float top, float zNear, float zFar)
	{
		return ortho(left, right, bottom, top, zNear, zFar);
	}

	void processKeyboard(Direction dir, float deltaTime, bool constrainY = false)
	{
		float velocity = speed * deltaTime;
		if (!constrainY)
		{
			if (dir == FORWARD) position += front * velocity;
			if (dir == BACKWARD) position -= front * velocity;
			if (dir == LEFT) position -= right * velocity;
			if (dir == RIGHT) position += right * velocity;
			if (dir == UP) position += up * velocity;
			if (dir == DOWN) position -= up * velocity;
		}
		else
		{
			if (dir == FORWARD) position += vec3(front.x, 0, front.z) * velocity;
			if (dir == BACKWARD) position -= vec3(front.x, 0, front.z) * velocity;
			if (dir == LEFT) position -= vec3(right.x, 0, right.z) * velocity;
			if (dir == RIGHT) position += vec3(right.x, 0, right.z) * velocity;
		}
	}

	void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true)
	{
		if (mouseMovementState) return;
		yaw += xoffset * sensitivity;
		pitch += yoffset * sensitivity;

		if (constrainPitch)
		{
			if (pitch > 89) pitch = 89;
			if (pitch < -89) pitch = -89;
		}

		updateCameraVector();
	}

	void processMouseScroll(float yoffset)
	{
		fov -= yoffset;
		if (fov > 45) fov = 45;
		if (fov < 1) fov = 1;
	}

private:
	void updateCameraVector()
	{
		front.x = cos(radians(pitch)) * cos(radians(yaw));
		front.y = sin(radians(pitch));
		front.z = cos(radians(pitch)) * sin(radians(yaw));
		right = normalize(cross(front, worldUp));
		up = normalize(cross(right, front));
	}
};