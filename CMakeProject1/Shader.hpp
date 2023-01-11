#ifndef Shader_hpp
#define Shader_hpp

#include <GL/glew.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <glm/glm.hpp>
namespace gps {

class Shader
{
public:
    GLuint shaderProgram;
    void loadShader(std::string vertexShaderFileName, std::string fragmentShaderFileName, std::string geometryShaderFileName = "");
    void useShaderProgram();
    void setVec3(const std::string& name, const glm::vec3& value) const
    {
        glUniform3fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, &value[0]);
    }
    void setVec3(const std::string& name, float x, float y, float z) const
    {
        glUniform3f(glGetUniformLocation(shaderProgram, name.c_str()), x, y, z);
    }
    // ------------------------------------------------------------------------
    void setVec4(const std::string& name, const glm::vec4& value) const
    {
        glUniform4fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, &value[0]);
    }
    void setVec4(const std::string& name, float& x, float& y, float& z, float& w)
    {
        glUniform4f(glGetUniformLocation(shaderProgram, name.c_str()), x, y, z, w);
    }
    void setMat3(const std::string& name, const glm::mat3& mat) const
    {
        glUniformMatrix3fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat4(const std::string& name, const glm::mat4& mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, GL_FALSE, &mat[0][0]);      
    }
    void setFloat(const std::string& name, const float& flt)
    {
        glUniform1f(glGetUniformLocation(shaderProgram, name.c_str()), flt);
    }
    void setInt(const std::string& name, const int& it)
    {
        glUniform1i(glGetUniformLocation(shaderProgram, name.c_str()), it);
    }

   
private:
    std::string readShaderFile(std::string fileName);
    void shaderCompileLog(GLuint shaderId);
    void shaderLinkLog(GLuint shaderProgramId);
};

}

#endif /* Shader_hpp */
