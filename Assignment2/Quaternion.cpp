#include "Quaternion.h"



Quaternion::Quaternion(float uS, glm::vec3& uV)
{
	this->s = uS;
	this->v = uV;
}


Quaternion::~Quaternion()
{
}
