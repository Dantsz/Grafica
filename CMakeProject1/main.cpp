#include <iostream>

#include "Shader.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "Shader.hpp"
#include "Camera.hpp"

#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"


int glWindowWidth = 1500;
int glWindowHeight = 800;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

gps::Model3D teapot;

glm::mat4 model;
GLuint modelLoc;
GLuint modelLoc2;
glm::mat4 view;
GLuint viewLoc;
GLuint viewLoc2;
glm::mat4 projection;
GLuint projectionLoc;
GLuint projectionLoc2;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
GLuint normalMatrixLoc2;

glm::vec3 lightDir;
GLuint lightDirLoc;
GLuint lightDirLoc2;
glm::vec3 lightColor;
GLuint lightColorLoc;
GLuint lightColorLoc2;
glm::vec3 baseColor(1.0f, 0.55f, 0.0f); //orange
GLuint baseColorLoc;
GLuint baseColorLoc2;
glm::vec3 viewPos;

gps::Camera myCamera(
	glm::vec3(0.0f, 0.0f, 3.0f),
	glm::vec3(0.0f, 0.0f, -10.0f),
	glm::vec3(0.0f, 1.0f, 0.0f)
);
GLfloat cameraSpeed = 0.05f;

bool pressedKeys[1024];
GLfloat angleY;

gps::Shader myCustomShader;
gps::Shader myCustomShader2;

GLenum glCheckError_(const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height)
{
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{

}

void processMovement()
{
	if (pressedKeys[GLFW_KEY_Q]) {
		angleY -= 1.0f;

		model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		myCustomShader2.useShaderProgram();
		glUniformMatrix4fv(modelLoc2, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix3fv(normalMatrixLoc2, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angleY += 1.0f;

		model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		myCustomShader2.useShaderProgram();
		glUniformMatrix4fv(modelLoc2, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix3fv(normalMatrixLoc2, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);

		view = myCamera.getViewMatrix();
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightDir));
		myCustomShader2.useShaderProgram();
		glUniformMatrix4fv(viewLoc2, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix3fv(normalMatrixLoc2, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUniform3fv(lightDirLoc2, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightDir));
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);

		view = myCamera.getViewMatrix();
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightDir));
		myCustomShader2.useShaderProgram();
		glUniformMatrix4fv(viewLoc2, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix3fv(normalMatrixLoc2, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUniform3fv(lightDirLoc2, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightDir));
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);

		view = myCamera.getViewMatrix();
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightDir));
		myCustomShader2.useShaderProgram();
		glUniformMatrix4fv(viewLoc2, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix3fv(normalMatrixLoc2, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUniform3fv(lightDirLoc2, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightDir));
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);

		view = myCamera.getViewMatrix();
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightDir));
		myCustomShader2.useShaderProgram();
		glUniformMatrix4fv(viewLoc2, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix3fv(normalMatrixLoc2, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUniform3fv(lightDirLoc2, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightDir));
	}
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

	glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	//glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

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

void initObjects()
{
	teapot.LoadModel("models/teapots/teapot4segU.obj", "models/teapots/");
}

void initUniforms()
{
	myCustomShader.useShaderProgram();

	model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	projection = glm::perspective(glm::radians(45.0f), ((float)retina_width / (float)2) / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	baseColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "baseColor");
	glUniform3fv(baseColorLoc, 1, glm::value_ptr(baseColor));

	myCustomShader2.useShaderProgram();

	modelLoc2 = glGetUniformLocation(myCustomShader2.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc2, 1, GL_FALSE, glm::value_ptr(model));

	viewLoc2 = glGetUniformLocation(myCustomShader2.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc2, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrixLoc2 = glGetUniformLocation(myCustomShader2.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc2, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	projectionLoc2 = glGetUniformLocation(myCustomShader2.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc2, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction
	lightDirLoc2 = glGetUniformLocation(myCustomShader2.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc2, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightDir));

	//set light color
	lightColorLoc2 = glGetUniformLocation(myCustomShader2.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc2, 1, glm::value_ptr(lightColor));

	baseColorLoc2 = glGetUniformLocation(myCustomShader2.shaderProgram, "baseColor");
	glUniform3fv(baseColorLoc2, 1, glm::value_ptr(baseColor));
}

void initShaders()
{
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
	myCustomShader2.loadShader("shaders/shaderPPL.vert", "shaders/shaderPPL.frag");
	myCustomShader2.useShaderProgram();
}

void initOpenGLState()
{
	glClearColor(0.8, 0.8, 0.8, 1.0);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing	
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"

	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void renderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, retina_width / 2, retina_height);
	teapot.Draw(myCustomShader);
	glViewport(retina_width / 2, 0, retina_width / 2, retina_height);
	teapot.Draw(myCustomShader2);
}

void cleanup() {

	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

int main(int argc, const char* argv[]) {

	initOpenGLWindow();
	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();

	while (!glfwWindowShouldClose(glWindow)) {
		processMovement();
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	cleanup();

	return 0;
}
