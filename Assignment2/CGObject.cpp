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

	glm::mat4 CGObject::createTransform()
	{
		glm::mat4 localTransform = glm::mat4(1.0);
		
		// Overall Euler angle
		//glm::mat4 matrix1 = glm::eulerAngleXYZ(this->eulerAngles.x, this->eulerAngles.y, this->eulerAngles.z); //pitch, yaw, roll);
		HMatrix R;
		EulerAngles ea = { this ->eulerAngles.x, this->eulerAngles.y, this->eulerAngles.z, EulOrdXYZr };
		Eul_ToHMatrix(ea, R);
		glm::mat4 R_glm = glm::mat4(R[0][0], R[0][1], R[0][2], R[0][3],
									R[1][0], R[1][1], R[1][2], R[1][3], 
									R[2][0], R[2][1], R[2][2], R[2][3], 
									R[3][0], R[3][1], R[3][2], R[3][3]);
		
					   
		localTransform = glm::translate(localTransform, this->position);
		//localTransform = localTransform * R_glm ;

		localTransform = glm::scale(localTransform, this->initialScaleVector);

		glm::mat4 parentTransform = Parent == nullptr ? glm::mat4(1.0) : Parent->globalTransform;
		return parentTransform * localTransform;
	}
}