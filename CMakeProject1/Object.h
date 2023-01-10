#pragma once
#include "Model3D.hpp"
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/type_ptr.hpp>
class Object
{
public:
	Object(const std::shared_ptr<gps::Model3D>& model):
		model{model},
		angle{0.0f},
		model_mat{ glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f)) }
	{

	}
	void render(gps::Shader& shader, const glm::mat4&  view,bool depth_pass = false) ;
	
	virtual void setPosition(const glm::vec3& pos)
	{
		model_mat = glm::translate(glm::mat4(1.0f), pos);
		model_mat = glm::rotate(model_mat, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
		model_mat = glm::scale(model_mat, scale);
	}

	void rotate(GLfloat added_angle, glm::vec3 axis)
	{
		angle += added_angle;
		model_mat = glm::rotate(model_mat, glm::radians(added_angle),axis);
	}
	void rotate_fixed(GLfloat angle, glm::vec3 axis)
	{
		angle += angle;
		model_mat = glm::rotate(model_mat, glm::radians(angle), axis);
	}
	virtual void update()
	{

	}
	virtual void set_scale(const glm::vec3& new_scale)
	{
		scale = new_scale;
		model_mat = glm::scale(model_mat,new_scale) ;
	}

	GLfloat angle;
	glm::mat4 model_mat;
	glm::mat3 normal_matrix{};
	std::shared_ptr<gps::Model3D> model;
	glm::vec3 scale = glm::vec3(1.0f);

};
