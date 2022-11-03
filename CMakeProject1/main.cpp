#include <iostream>

#include "Shader.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

int glWindowWidth = 640;
int glWindowHeight = 480;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;
bool draw_wireframe = false;
//vertex coordinates in normalized device coordinates
GLfloat vertexData[] = {
         0.0f, 0.7f,    0.0f,     1.0f, 0.0f, 0.0f,
        -0.5f,	0.5f,	0.0f,  0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f,	0.0f,  1.0f, 0.0f, 1.0f,
         0.5f, -0.5f,	0.0f,  1.0f, 0.0f, 1.0f,
         0.5f,  0.5f,   0.0f,  1.0f, 1.0f, 0.0f,
};
GLuint triangleIndices[] = 
{ 0,1,4, 
  1,2,3,
  1,3,4
};

GLuint verticesVBO;
GLuint objectVAO;
GLuint objectEBO;
gps::Shader myCustomShader;

void windowResizeCallback(GLFWwindow* window, int width, int height)
{
    fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
    //TODO
}

void initObjects()
{
    glGenVertexArrays(1, &objectVAO);
    glBindVertexArray(objectVAO);

    glGenBuffers(1, &verticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, verticesVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

    glGenBuffers(1, &objectEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,  objectEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangleIndices), triangleIndices, GL_STATIC_DRAW);

    //vertex position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);
    //
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

bool initOpenGLWindow()
{
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // for multisampling/antialising
    glfwWindowHint(GLFW_SAMPLES, 4);

    glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
    if (!glWindow) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        return false;
    }

    glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
    glfwMakeContextCurrent(glWindow);

    // start GLEW extension handler
    glewExperimental = GL_TRUE;
    glewInit();

    // get version info
    const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
    const GLubyte* version = glGetString(GL_VERSION); // version as a string
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);

    //for RETINA display
    glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

    return true;
}

void renderScene()
{
    static float x_movement = 0.0f;
    glClearColor(0.8, 0.8, 0.8, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glViewport(0, 0, retina_width, retina_height);

    if (glfwGetKey(glWindow, GLFW_KEY_A)) {
        x_movement += 0.001f;
        GLint camera_uniform = glGetUniformLocation(myCustomShader.shaderProgram, "camera_position");
        glUniform3f(camera_uniform, x_movement, 0.0f, 0.0);
    }

    if (glfwGetKey(glWindow, GLFW_KEY_D)) {
        x_movement -= 0.001f;
        GLint camera_uniform = glGetUniformLocation(myCustomShader.shaderProgram, "camera_position");
        glUniform3f(camera_uniform, x_movement, 0.0f, 0.0);
    }
    if (glfwGetKey(glWindow, GLFW_KEY_T))
    {
        draw_wireframe = !draw_wireframe;
        if (draw_wireframe)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }
    if (glfwGetKey(glWindow, GLFW_KEY_ESCAPE))
    {
        glfwSetWindowShouldClose(glWindow, GLFW_TRUE);
    }
    
    glBindVertexArray(objectVAO);
    myCustomShader.useShaderProgram();

    glDrawElements(GL_TRIANGLES, 9,GL_UNSIGNED_INT, 0);
   
}

void cleanup() {
    glfwDestroyWindow(glWindow);
    //close GL context and any other GLFW resources
    glfwTerminate();
}

int main(int argc, const char* argv[]) {

    if (!initOpenGLWindow()) {
        glfwTerminate();
        return 1;
    }

    initObjects();

    myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
    myCustomShader.useShaderProgram();

    while (!glfwWindowShouldClose(glWindow)) {
        renderScene();

        glfwPollEvents();
        glfwSwapBuffers(glWindow);
    }

    cleanup();

    return 0;
}