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

	glm::mat4 CGObject::createTransform()
	{
		glm::mat4 localTransform = glm::mat4(1.0);
									   
		localTransform = glm::translate(localTransform, this->position);

		localTransform = glm::rotate(localTransform, this->eulerAngles.z, glm::vec3(0, 0, 1));
		localTransform = glm::rotate(localTransform, this->eulerAngles.y, glm::vec3(0, 1, 0));
		localTransform = glm::rotate(localTransform, this->eulerAngles.x, glm::vec3(1, 0, 0));

		localTransform = glm::scale(localTransform, this->initialScaleVector);

		glm::mat4 parentTransform = Parent == nullptr ? glm::mat4(1.0) : Parent->globalTransform;
				
		return parentTransform * localTransform;
	}
}