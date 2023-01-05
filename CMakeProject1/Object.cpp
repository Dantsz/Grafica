#include "Object.h"

void Object::render(gps::Shader& shader, const glm::mat4& view, bool depth_pass)
{
    //send teapot model matrix data to shader
    shader.setMat4("model",model_mat);
    //send teapot normal matrix data to shader
  
    if (!depth_pass)
    {      
        normal_matrix = glm::mat3(glm::inverseTranspose(model_mat));
        shader.setMat3("normalMatrix", normal_matrix);
    }
    model->Draw(shader);
} 
