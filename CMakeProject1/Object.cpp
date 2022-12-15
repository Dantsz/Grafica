#include "Object.h"

void Object::render(gps::Shader& shader, const glm::mat4& view, bool depth_pass)
{
    shader.useShaderProgram();

    //send teapot model matrix data to shader
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model_mat));

    //send teapot normal matrix data to shader
    normal_matrix = glm::mat3(glm::inverseTranspose(view * model_mat));
    if (!depth_pass)
    {   
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normal_matrix));
    }
    model->Draw(shader);
} 
