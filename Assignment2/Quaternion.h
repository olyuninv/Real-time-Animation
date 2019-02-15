#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Quaternion
{
public:
	
	float s;   //scalar	
	glm::vec3 v;   //vector

	Quaternion(float uS, glm::vec3& uV);
	~Quaternion();
};

