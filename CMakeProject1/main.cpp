
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"
#include "Object.h"
#include <iostream>
#include "imgui.h"

#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


// window
gps::Window myWindow;
int retina_width, retina_height;
// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;


// light parameters
glm::vec3 lightDir = {0.0f, 1.0f, 1.0f};
glm::vec3 lightVec;
glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light


// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;

// camera
gps::Camera myCamera(glm::vec3(0.0f, 5.0f, 15.0f), glm::vec3(0.0f, 2.0f, -10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
constexpr float render_distance = 2000.0f;
GLfloat cameraSpeed = 0.1f;

GLboolean pressedKeys[1024];

// models
std::shared_ptr<gps::Model3D> teapot_model = std::make_shared<gps::Model3D>();
std::shared_ptr<gps::Model3D> debris_model = std::make_shared<gps::Model3D>();
std::shared_ptr<gps::Model3D> ground_model = std::make_shared<gps::Model3D>();
std::shared_ptr<gps::Model3D> dust2_model = std::make_shared<gps::Model3D>();
std::shared_ptr<gps::Model3D> sponza_model = std::make_shared<gps::Model3D>();
GLfloat angle;

// shaders
gps::Shader myBasicShader;
gps::Shader depthMapShader;
//teapot object
bool cursor = true;
float lightAngle = 0.0f;

glm::mat4 lightMatrixTR;
//generate FBO ID
GLuint shadowMapFBO;
GLuint depthMapTexture;
const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;
GLfloat lightSpaceTrMatrix_near_plane = 0.1f, lightSpaceTrMatrix_far_plane = 5.0f;
glm::vec3 lightEye{ 0.0f };
glm::vec4 shadow_projection_coord = { -10.0f, 10.0f, -10.0f, 10.0f };
std::vector<Object> objects{};
//skybox
std::vector<const GLchar*> faces;
gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                error = "STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
	//TODO
}
void initSkybox()
{
  
    faces.push_back("skybox/alps_rt.tga");
    faces.push_back("skybox/alps_lf.tga");
    faces.push_back("skybox/alps_up.tga");
    faces.push_back("skybox/alps_dn.tga");
    faces.push_back("skybox/alps_bk.tga");
    faces.push_back("skybox/alps_ft.tga");
    mySkyBox.Load(faces);
    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
}
void initFBO() {
    glGenFramebuffers(1, &shadowMapFBO);
    //TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
    //create depth texture for FBO
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    // attach texture to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture,
        0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    static bool firstMouse = true;
    static float lastX = myWindow.getWindowDimensions().width  / 2.0f;
    static float lastY = myWindow.getWindowDimensions().height / 2.0f;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xOffset = xpos - lastX;
    float yOffset = ypos - lastY;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.01f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;
    if (cursor)
    {
        myCamera.rotate(yOffset, xOffset);
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    }
}

void processMovement() {
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		//update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	}

    if (pressedKeys[GLFW_KEY_Q]) {
        objects[0].rotate(-1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    if (pressedKeys[GLFW_KEY_E]) {
        objects[0].rotate(1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    }
}

void initOpenGLWindow() {
    myWindow.Create(1920, 768, "OpenGL Project Core");
}
void initImGuiContext(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
}
void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_O)
        {
            
            myWindow.setEnableCursor(cursor, []() {}, []() {});
            cursor = !cursor;
        }
    }
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        }
        else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}
void setWindowCallbacks() { 
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise   
}

void initModels() {
    teapot_model->LoadModel("models/teapot/teapot20segUT.obj");
    debris_model->LoadModel("models/debris/debris.obj");
    ground_model->LoadModel("models/ground/ground.obj");
    dust2_model->LoadModel("models/cluck/untitled.obj");
    sponza_model->LoadModel("models/Sponza/sponza.obj");
}

void initShaders() {
	myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");
    depthMapShader.loadShader("shaders/shadow.vert", "shaders/shadow.frag");
  
}
glm::mat4 computeLightSpaceTrMatrix() {
    //TODO - Return the light-space transformation matrix
    glm::vec3 ld = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 0.0f));
    glm::mat4 lightView = glm::lookAt(ld, lightEye, glm::vec3(0.0f, 1.0f, 0.0f));
  
    glm::mat4 lightProjection = glm::ortho(shadow_projection_coord[0], shadow_projection_coord[1], shadow_projection_coord[2], shadow_projection_coord[3], lightSpaceTrMatrix_near_plane, lightSpaceTrMatrix_far_plane);
    glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
    return lightSpaceTrMatrix;
}

void initUniforms() {
	myBasicShader.useShaderProgram();

	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	// create projection matrix
	projection = glm::perspective(glm::radians(45.0f),
                               (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
                               0.1f, render_distance);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	//set the light direction (direction towards the light)
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	// send light dir to shader
	

	//set light color
	
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
}


void renderScene() {
	//SHADOW
    depthMapShader.useShaderProgram();
    lightMatrixTR = computeLightSpaceTrMatrix();
    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(lightMatrixTR));
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);

    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);
    for ( auto& object : objects)
    {
        object.render(depthMapShader, view, true);
    }
    glCullFace(GL_BACK);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //SKYBOX
    glViewport(0, 0, retina_width, retina_height);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    skyboxShader.useShaderProgram();
    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
        glm::value_ptr(view));

    projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE,
        glm::value_ptr(projection));

    mySkyBox.Draw(skyboxShader, view, projection);

    //NORMAL
    myBasicShader.useShaderProgram();

    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    const auto lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    lightVec = glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir;
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightVec));

    //bind the shadow map
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 3);

    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(lightMatrixTR));
    for ( auto& object : objects)
    {
        object.render(myBasicShader, view);
    }

}

void cleanup() {
    myWindow.Delete();
}


int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
    setWindowCallbacks();
    initImGuiContext(myWindow.getWindow());
  

	initModels();
	initShaders();
	initUniforms();
    initFBO();
    initSkybox();
    glfwGetFramebufferSize(myWindow.getWindow(), &retina_width, &retina_height);
	glCheckError();
	// application loop
    glm::vec3 ps = { 0.0f,1.0f,0.0f };
    glm::vec3 ps2 = { 0.0f,-100.0f,0.0f };
    glm::vec3 ground_ps{ 0.0f,0.0f,0.0f };
    objects.emplace_back(teapot_model);
    objects.emplace_back(ground_model);
    objects.emplace_back(debris_model);
    objects.emplace_back(dust2_model);
    objects.emplace_back(teapot_model);
    objects.emplace_back(sponza_model);
    objects[objects.size() - 1].set_scale({ 0.1f,0.1f,0.1f });
    objects[3].setPosition({0,-39,0});
  
    objects[1].setPosition(ground_ps);
    objects[2].setPosition(ps2);
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Stats");
            ImGui::InputFloat3("Position",   glm::value_ptr(myCamera.cameraPosition));
            ImGui::InputFloat3("Camera target",    glm::value_ptr(myCamera.cameraTarget));
            ImGui::InputFloat3("Front direction",  glm::value_ptr(myCamera.cameraFrontDirection));
            ImGui::InputFloat3("Right direction",  glm::value_ptr(myCamera.cameraRightDirection));
            ImGui::InputFloat3("UP direction",     glm::value_ptr(myCamera.cameraUpDirection));
            ImGui::Separator();
        ImGui::End();
        ImGui::Begin("Global light");
            ImGui::DragFloat3("Direction", glm::value_ptr(lightDir),0.1,-1.0,1.0);
            ImGui::InputFloat("Angle", &lightAngle, 1.f);
            ImGui::InputFloat3("Light Eye", glm::value_ptr(lightEye));
            ImGui::InputFloat("Light Space Tr Matrix Near_plane", &lightSpaceTrMatrix_near_plane,0.05f);
            ImGui::InputFloat("Light Space Tr Matrix Far_plane", &lightSpaceTrMatrix_far_plane, 0.05f);
            ImGui::InputFloat4("shadow_projection_coord:", glm::value_ptr(shadow_projection_coord));
        ImGui::End();
        ImGui::Begin("Teapot position");
            ImGui::InputFloat("x:",&ps.x,0.1f,1.0f);
            ImGui::InputFloat("y:", &ps.y, 0.1f, 1.0f);
            ImGui::InputFloat("z:", &ps.z, 0.1f, 1.0f);
        ImGui::End();
        ImGui::Begin("Rendering");
        if (ImGui::CollapsingHeader("Polygon Mode"))
        {
            if (ImGui::Button("WireFrame")) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            if (ImGui::Button("Normal")) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
        }
         
        ImGui::End();

        processMovement();
      
   
        objects[0].setPosition(ps);

  
	    renderScene();
              glCheckError();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glCheckError();
		glfwSwapBuffers(myWindow.getWindow());
        glCheckError();
	}
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
	cleanup();

    return EXIT_SUCCESS;
}
