//
//  Shader.hpp
//  Lab3
//
//  Created by CGIS on 05/10/2016.
//  Copyright © 2016 CGIS. All rights reserved.
//

#ifndef Shader_hpp
#define Shader_hpp


#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
//
//  main.cpp
//  OpenGL_Shader_Example_step1
//
//  Created by CGIS on 30/11/15.
//  Copyright © 2015 CGIS. All rights reserved.
//


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

namespace gps {
    
class Shader
{
public:
    GLuint shaderProgram;
    void loadShader(std::string vertexShaderFileName, std::string fragmentShaderFileName);
    void useShaderProgram();
    
private:
    std::string readShaderFile(std::string fileName);
    void shaderCompileLog(GLuint shaderId);
    void shaderLinkLog(GLuint shaderProgramId);
};
    
}

#endif /* Shader_hpp */
