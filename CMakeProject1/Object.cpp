#include "Object.h"

void Object::render(gps::Shader shader, const glm::mat4& view, GLint modelLoc, GLint normalLoc)
{
    shader.useShaderProgram();

    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model_mat)); 

    //send teapot normal matrix data to shader
    normal_matrix = glm::inverseTranspose(view * model_mat);
    glUniformMatrix3fv(normalLoc, 1, GL_FALSE, glm::value_ptr(normal_matrix));

    // draw teapot
    model->Draw(shader);
} 
