#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

const int GL_WINDOW_WIDTH = 640;
const int GL_WINDOW_HEIGHT = 480;

GLFWwindow* glWindow = NULL;

glm::vec4 clearColor{ 1.0f, 1.0f, 1.0f, 1.0f };

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glWindow = glfwCreateWindow(GL_WINDOW_WIDTH, GL_WINDOW_HEIGHT, "Hello Window", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(glWindow);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	return true;
}
void cleanup() {
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

void renderScene() {
	//TODO drawing code
	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT);
}

int main(int argc, const char* argv[]) {

	initOpenGLWindow();

	while (!glfwWindowShouldClose(glWindow)) {

		renderScene();

		glfwSwapBuffers(glWindow);
		glfwPollEvents();
	}

	cleanup();
	return 0;
}