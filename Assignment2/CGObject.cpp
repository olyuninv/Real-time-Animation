#include "CGObject.h"

namespace Assignment2
{
	CGObject::CGObject()
	{
	}

	CGObject::~CGObject()
	{
	}

	void CGObject::Draw(opengl_utils glutils)
	{
		int VBOindex = this->startVBO;
		int IBOindex = this->startIBO;
		for (int i = 0; i < this->Meshes.size(); i++) {

			glutils.linkCurrentBuffertoShader(this->VAOs[i], VBOindex, IBOindex);

			glDrawElements(GL_TRIANGLES, this->Meshes[i].Indices.size(), GL_UNSIGNED_INT, (void*)(IBOindex * sizeof(unsigned int)));
			VBOindex += Meshes[i].Vertices.size();
			IBOindex += Meshes[i].Indices.size();
		}
	}

	//glm::mat4 yawPitchRollCustom 
	//(
	//	float yaw,
	//	float pitch,
	//	float roll
	//)
	//{
	//	float tmp_ch = glm::cos(yaw);
	//	float tmp_sh = glm::sin(yaw);
	//	float tmp_cp = glm::cos(pitch);
	//	float tmp_sp = glm::sin(pitch);
	//	float tmp_cb = glm::cos(roll);
	//	float tmp_sb = glm::sin(roll);

	//	glm::mat4 Result;
	//	Result[0][0] = tmp_ch * tmp_cb;
	//	Result[0][1] = tmp_ch * tmp_sb * tmp_sp - tmp_sh * tmp_cp;
	//	Result[0][2] = tmp_ch* tmp_sb * tmp_cp + tmp_sh * tmp_sp;
	//	Result[0][3] = 0.0f;
	//	Result[1][0] = tmp_sh* tmp_sb;
	//	Result[1][1] = tmp_sh* tmp_sb*tmp_sp + tmp_ch * tmp_cp;
	//	Result[1][2] = tmp_sh * tmp_sb * tmp_cp - tmp_ch * tmp_cp;
	//	Result[1][3] = 0.0f;
	//	Result[2][0] = -tmp_sb;
	//	Result[2][1] = tmp_cb* tmp_sp;
	//	Result[2][2] = tmp_cb * tmp_cp;
	//	Result[2][3] = 0.0f;
	//	Result[3][0] = 0.0f;
	//	Result[3][1] = 0.0f;
	//	Result[3][2] = 0.0f;
	//	Result[3][3] = 1.0f;
	//	return Result;
	//}

	void CGObject::setInitialRotation(glm::vec3 initialRotationEuler)
	{
		this->initialRotateAngleEuler = initialRotationEuler;
		this->eulerAngles = initialRotationEuler;
		this->rotateAngles = initialRotationEuler;
		glm::quat initialQuatRotation = glm::quat(this->initialRotateAngleEuler);
		glm::mat4 rotationMatrix = glm::toMat4(initialQuatRotation);

		this->orientation = glm::mat4(rotationMatrix);
	}

	glm::mat4 CGObject::createTransform(bool isRotationQuaternion)
	{
		glm::mat4 localTransform = glm::mat4(1.0);

		localTransform = glm::translate(localTransform, this->position);

		//glm::mat4 rotationMatrix = glm::EulerAngles(this->eulerAngles.x, this->eulerAngles.y, this->eulerAngles.z);

		glm::mat4 rotationMatrix = glm::mat4(1.0);

		if (isRotationQuaternion)
		{
			glm::vec3 newRotation = glm::vec3(this->eulerAngles.x - this->rotateAngles.x,
				this->eulerAngles.y - this->rotateAngles.y,
				this->eulerAngles.z - this->rotateAngles.z);

			glm::quat quaternionRotation = glm::quat(newRotation);
			rotationMatrix = this->orientation * glm::toMat4(quaternionRotation);

			this->rotateAngles = glm::vec3(this->eulerAngles.x, this->eulerAngles.y, this->eulerAngles.z);
			this->orientation = glm::mat4( rotationMatrix);

		}
		else
		{
			rotationMatrix = glm::rotate(rotationMatrix, this->eulerAngles.y, glm::vec3(0, 1, 0));
			rotationMatrix = glm::rotate(rotationMatrix, this->eulerAngles.z, glm::vec3(0, 0, 1));
			rotationMatrix = glm::rotate(rotationMatrix, this->eulerAngles.x, glm::vec3(1, 0, 0));

			//rotationMatrix = glm::yawPitchRoll(this->eulerAngles.y, this->eulerAngles.x, this->eulerAngles.z);
			//rotationMatrix = glm::eulerAngleXYZ(this->eulerAngles.x, this->eulerAngles.y, this->eulerAngles.z);
		}

		localTransform = localTransform * rotationMatrix;

		localTransform = glm::scale(localTransform, this->initialScaleVector);

		glm::mat4 parentTransform = Parent == nullptr ? glm::mat4(1.0) : Parent->globalTransform;

		return parentTransform * localTransform;
	}
}