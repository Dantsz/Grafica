
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types
#include <btBulletDynamicsCommon.h>


#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"
#include "Object.h"
#include <iostream>
#include "imgui.h"
#include <array>
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

import GravityObject;
// window
gps::Window myWindow;
int retina_width, retina_height;
// matrices
glm::mat4 view;
glm::mat4 projection;




glm::vec3 lightColor = glm::vec3(1.0f, 1.0f,1.0f); //white light



// camera
gps::Camera myCamera(glm::vec3(0.0f, 5.0f, 15.0f), glm::vec3(0.0f, 2.0f, -10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
constexpr float render_distance = 300;
GLfloat cameraSpeed = 0.1f;

GLboolean pressedKeys[1024];

// models
std::shared_ptr<gps::Model3D> teapot_model = std::make_shared<gps::Model3D>();
std::shared_ptr<gps::Model3D> debris_model = std::make_shared<gps::Model3D>();
std::shared_ptr<gps::Model3D> ground_model = std::make_shared<gps::Model3D>();
std::shared_ptr<gps::Model3D> dust2_model  = std::make_shared<gps::Model3D>();
std::shared_ptr<gps::Model3D> sponza_model = std::make_shared<gps::Model3D>();
GLfloat angle;

// shaders
gps::Shader myBasicShader;
gps::Shader depthMapShader;
//teapot object
bool cursor = true;


glm::mat4 lightMatrixTR;
glm::mat3 normalMatrix;
//generate FBO ID


const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;


std::vector<std::unique_ptr<Object>> objects{};
//skybox
std::vector<const GLchar*> faces;
gps::SkyBox mySkyBox;
gps::Shader skyboxShader;
//point light
float constant = 1.f;
float linear = 0.09f;
float quadratic = 0.032f;
//physics
 btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
 btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
 btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();
 btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();
 btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
 glm::vec3 teapot_factory_delivery_position = glm::vec3(0, 5, 0);
 float teapot_mass = 1.f;
 float teapot_scale = 1.f;
 glm::vec3 wind_direction = glm::vec3(1, 1, 1);
    //
std::array<glm::vec3, 4> pointLightPositions = {
    glm::vec3(-112.0f,  11.41f,  -40.0f),
    glm::vec3(-137.995f,  18.931f,  -6.329f),
    glm::vec3(111.0f,  0.0F,  0.0f),
    glm::vec3(-138.808f,  18.380F,  -1.111f),
};
std::array<glm::vec3,4> pointLightColor = {
    glm::vec3(0.f,  0,  1.f),
    glm::vec3(0.0f,  0,  1.0f),
    glm::vec3(10.0f,  10.0,  0.f),
    glm::vec3(1.0,  0.0f,  0.0f)
};

//for point shadows
unsigned int depthCubemap;
unsigned int depthMapFBO;
//for Bloom
unsigned int hdrFBO;
unsigned int colorBuffers[2];
unsigned int pingpongFBO[2];
unsigned int pingpongBuffer[2];
gps::Shader shaderBlur;

unsigned int quadVAO = 0;
unsigned int quadVBO;

bool bloom = true;
bool gamma_corretion = false;
float exposure = 1.0f;

gps::Shader bloomMerge;


void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

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

glm::vec3 lightPos(0.0f, 1.0f, 0.0f);
float near_plane = 1.0f;
float far_plane = 400.0f;
glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
std::vector<glm::mat4> shadowTransforms;


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
    //create point shadow stuff
    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    shadowTransforms.resize(6);
    
    //FBO FOR BLOOM
    glCheckError();
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    
    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA16F, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height, 0, GL_RGBA, GL_FLOAT, NULL
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach texture to framebuffer
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0
        );
        unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

        glDrawBuffers(2, attachments);
       
    }
    unsigned int depthBuffer;
    glGenTextures(1, &depthBuffer);
    glBindTexture(GL_TEXTURE_2D, depthBuffer);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBuffer, 0
    );

    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongBuffer);
    for (unsigned int i = 0; i < 2; i++)
    {
      
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glCheckError();
        glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA16F, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height, 0, GL_RGBA, GL_FLOAT, NULL
        );
        glCheckError();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glCheckError();
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffer[i], 0
        );
    }
    glCheckError();
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

    float sensitivity = 0.05f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;
    
    if (cursor)
    {
        myCamera.rotate(yOffset, xOffset);        
    }
}

void processMovement() {
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
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
    ImGuiIO& io = ImGui::GetIO();
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
    //glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
    glEnable(GL_CULL_FACE); // cull face
    glCullFace(GL_BACK); // cull back face
    glFrontFace(GL_CCW); // GL_CCW for counter clock-wise   
}


void initModels() {
    teapot_model->LoadModel("models/teapot/teapot20segUT.obj");
    sponza_model->LoadModel("models/Sponza/sponza.obj");
}

void initShaders() {
	myBasicShader.loadShader("shaders/basic.vert","shaders/basic.frag");
    depthMapShader.loadShader ("shaders/shadow.vert", "shaders/shadow.frag","shaders/shadow.geo");
    shaderBlur.loadShader("shaders/gaussian.vert", "shaders/gaussian.frag");
    bloomMerge.loadShader("shaders/finalbloom.vert","shaders/finalbloom.frag");
}

void initUniforms() {
	myBasicShader.useShaderProgram();


	projection = glm::perspective(glm::radians(45.0f),
                               (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
                               0.1f, render_distance);
    myBasicShader.setMat4("projection", projection);
    myBasicShader.setVec3("lightColor", lightColor);
	
}



void renderScene() {
	//SHADOW
    depthMapShader.useShaderProgram();
    shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
    shadowTransforms[0] = (shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms[1] = (shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms[2] = (shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
    shadowTransforms[3] = (shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
    shadowTransforms[4] = (shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms[5] = (shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
   
    for (unsigned int i = 0; i < 6; ++i)
        depthMapShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
    depthMapShader.setFloat("far_plane", far_plane);
    depthMapShader.setVec3("lightPos", lightPos);
    for ( auto& object : objects)
    {
        object->render(depthMapShader, view, true);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //SKYBOX
    glViewport(0, 0, retina_width, retina_height);

   

    skyboxShader.useShaderProgram();
    view = myCamera.getViewMatrix();
    projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
    mySkyBox.Draw(skyboxShader, view, projection);

    //NORMAL
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    myBasicShader.useShaderProgram();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    view = myCamera.getViewMatrix();
    myBasicShader.setMat4("view", view);

    //bind the shadow map
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "depthMap"), 3);

    myBasicShader.setVec3("viewPos", myCamera.cameraPosition);
    myBasicShader.setMat4("lightSpaceTrMatrix", lightMatrixTR);
    myBasicShader.setFloat("far_plane", far_plane);
    myBasicShader.setVec3("lightPos", lightPos);
    for (size_t i = 0; i < pointLightPositions.size(); i++)
    {
        myBasicShader.setVec3("pointLights[" + std::to_string(i) + "].position", pointLightPositions[i]);
        myBasicShader.setVec3("pointLights[" + std::to_string(i) + "].color", pointLightColor[i]);
        myBasicShader.setVec3("pointLights[" + std::to_string(i) + "].ambient", pointLightColor[i].x * 0.1, pointLightColor[i].y * 0.1, pointLightColor[i].z * 0.1);
        myBasicShader.setVec3("pointLights[" + std::to_string(i) + "].diffuse", pointLightColor[i].x, pointLightColor[i].y, pointLightColor[i].z);
        myBasicShader.setVec3("pointLights[" + std::to_string(i) + "].specular", pointLightColor[i].x, pointLightColor[i].y, pointLightColor[i].z);
        myBasicShader.setFloat("pointLights[" + std::to_string(i) + "].constant", constant);
        myBasicShader.setFloat("pointLights[" + std::to_string(i) + "].linear", linear);
        myBasicShader.setFloat("pointLights[" + std::to_string(i) + "].quadratic", quadratic);
    }
     
    
    for ( auto& object : objects)
    {
        object->render(myBasicShader, view);
    }

    bool horizontal = true, first_iteration = true;
    int amount = 10;
    shaderBlur.useShaderProgram();
    for (unsigned int i = 0; i < amount; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
        shaderBlur.setInt("horizontal", horizontal);
        glBindTexture(
            GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongBuffer[!horizontal]
        );
        renderQuad();
        horizontal = !horizontal;
        if (first_iteration)
            first_iteration = false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    bloomMerge.useShaderProgram();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pingpongBuffer[!horizontal]);
    bloomMerge.setInt("bloom", bloom);
    bloomMerge.setInt("gamma", gamma_corretion);
    bloomMerge.setFloat("exposure", exposure);
    renderQuad();
}

void cleanup() {
    myWindow.Delete();
}

void emplace_teapot_gently(glm::vec3 position, float teapot_mass, float scale)
{
    objects.emplace_back(std::make_unique<GravityObject>(teapot_model, btScalar(.20f), teapot_mass));

    GravityObject* teapot = dynamic_cast<GravityObject*>(objects[objects.size() - 1].get());
    dynamicsWorld->addRigidBody(teapot->getHitbox());
    teapot->getHitbox()->setFriction(0.5);
    teapot->getHitbox()->setRollingFriction(.5f);
    //teapot->getHitbox()->setDamping(.5,.5);
    btTransform tr;
    tr.setIdentity();
    btQuaternion quat;
    quat.setEuler(45, 45, 45); //or quat.setEulerZYX depending on the ordering you want
    tr.setRotation(quat);
    teapot->getHitbox()->getWorldTransform().setRotation(quat);

    teapot->getHitbox()->setAngularFactor(0.5);
    teapot->getHitbox()->setSpinningFriction(1.1);
    teapot->getHitbox()->setAnisotropicFriction(teapot->getShape()->getAnisotropicRollingFrictionDirection(), btCollisionObject::CF_ANISOTROPIC_ROLLING_FRICTION);
    teapot->setPosition(position);
    glm::vec3 scl = glm::vec3(scale, scale, scale);
    teapot->set_scale(scl);
}

float delta = 0;
float movementSpeed = 2; // units per second
void updateDelta(double elapsedSeconds) {
    delta  = elapsedSeconds;
}
double lastTimeStamp = glfwGetTime();

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
   
    
    dynamicsWorld->setGravity(btVector3(0, -10, 0));
    btDefaultMotionState* ground_motion_state = nullptr;
    btRigidBody* ground_body = nullptr;
    btStaticPlaneShape* ground_shape = nullptr;


    btAlignedObjectArray<btCollisionShape*> collisionShapes;
    {
        // btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(50.), btScalar(50.), btScalar(50.)));
        btVector3 planeNormal = btVector3(0.0f, 1.0f, 0.0f);
        ground_shape = new btStaticPlaneShape(planeNormal, 0);
        collisionShapes.push_back(ground_shape);

        btTransform groundTransform;
        groundTransform.setIdentity();
        groundTransform.setOrigin(btVector3(0, 0, 0));

        btScalar mass(0.);

        //rigidbody is dynamic if and only if mass is non zero, otherwise static
        bool isDynamic = (mass != 0.f);

        btVector3 localInertia(0, 0, 0);
        if (isDynamic)
            ground_shape->calculateLocalInertia(mass, localInertia);

        //using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
        ground_motion_state = new btDefaultMotionState(groundTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, ground_motion_state, ground_shape, localInertia);

        ground_body = new btRigidBody(rbInfo);
   
        ground_body->setRollingFriction(0.5);
        ground_body->setSpinningFriction(0.1);
        ground_body->setAnisotropicFriction(ground_shape->getAnisotropicRollingFrictionDirection(), btCollisionObject::CF_ANISOTROPIC_ROLLING_FRICTION);
        //add the body to the dynamics world
        dynamicsWorld->addRigidBody(ground_body);
    }

    initOpenGLState();
    setWindowCallbacks();
    initImGuiContext(myWindow.getWindow());
  

	initModels();
	initShaders();
    glCheckError();
	initUniforms();
    glCheckError();
    initFBO();
    glCheckError();
    initSkybox();
    glCheckError();
    glfwGetFramebufferSize(myWindow.getWindow(), &retina_width, &retina_height);
	glCheckError();
	// application loop
    

    for (float i = 0; i < 10; i++)
    {
        emplace_teapot_gently(glm::vec3(2.0f + i / 10.0f, i + 10.0f, +i / 100.0f),0.5f,1.0f);
    }

    objects.emplace_back(std::make_unique<Object>(sponza_model));

    objects[objects.size() - 1]->set_scale({ 0.1f,0.1f,0.1f });

	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        // get current time
        const double currentTimeStamp = glfwGetTime();
        updateDelta(currentTimeStamp - lastTimeStamp);
        lastTimeStamp = currentTimeStamp;
        
     
        dynamicsWorld->stepSimulation(delta, 10);
        for (const auto& object : objects)
        {
            object->update();
        }

        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Stats");
            ImGui::InputFloat3("Position",   glm::value_ptr(myCamera.cameraPosition));
            ImGui::InputFloat3("Front direction",  glm::value_ptr(myCamera.cameraFrontDirection));
            ImGui::InputFloat3("Right direction",  glm::value_ptr(myCamera.cameraRightDirection));
            ImGui::InputFloat3("UP direction",     glm::value_ptr(myCamera.cameraUpDirection));
            ImGui::Separator();
        ImGui::End();
        ImGui::Begin("Global light");
            ImGui::InputFloat3("LightPos", glm::value_ptr(lightPos));
            ImGui::InputFloat("Near plane", &near_plane, 0.1f, 1.0f);
            ImGui::InputFloat("Far Plane", &far_plane, 0.1f, 1.0f);
        ImGui::End();
        ImGui::Begin("Teapot Factory");
        
            ImGui::InputFloat3("Teapot delivery position", glm::value_ptr(teapot_factory_delivery_position));
            ImGui::InputFloat("Mass", &teapot_mass, 0.1f, 1.0f);
            ImGui::InputFloat("Scale", &teapot_scale, 0.1f, 1.0f);
            if (ImGui::Button("Order teapot"))
            {
                emplace_teapot_gently(teapot_factory_delivery_position,teapot_mass, teapot_scale);
            }
            ImGui::InputFloat3("Wind", glm::value_ptr(wind_direction));
            ImGui::SameLine();
            if (ImGui::Button("Divine Wind"))
            {
               
                for (auto& object : objects)
                {
                    GravityObject* pot = dynamic_cast<GravityObject*>(object.get());
                    if (pot)
                    {             
                        pot->getHitbox()->activate(true);
                        pot->getHitbox()->applyCentralImpulse(btVector3(wind_direction.x, wind_direction.y, wind_direction.z));
                    }
                }
            }
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

            if (ImGui::Button("Bloom"))
            {
                bloom = !bloom;
            }
            if (ImGui::Button("Gamma Corection"))
            {
                gamma_corretion = !gamma_corretion;
            }
        } 
        if (ImGui::CollapsingHeader("Blending"))
        {
            if (ImGui::Button("Enable"))
            {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
            if (ImGui::Button("Disable"))
            {
                glDisable(GL_BLEND);
            }
        }
     
        ImGui::End();

        ImGui::Begin("Point lights");
        if (ImGui::CollapsingHeader("Positions"))
        {
            for (size_t i = 0; i < pointLightPositions.size() ; i++)
            {
                ImGui::InputFloat3( ("Pos" + std::to_string(i)).c_str(), glm::value_ptr(pointLightPositions[i]));
            }
        }
        if (ImGui::CollapsingHeader("Colors"))
        {
            for (size_t i = 0; i < pointLightColor.size(); i++)
            {
                ImGui::InputFloat3(("Col" + std::to_string(i)).c_str(), glm::value_ptr(pointLightColor[i]));
            }
        }
        ImGui::Separator();

        ImGui::InputFloat("Constant", &constant, 0.05f);
        ImGui::InputFloat("Linear", &linear, 0.05f);
        ImGui::InputFloat("Quadratic", &quadratic, 0.05f);
        ImGui::End();

        ImGui::Begin("Control");
        if (ImGui::Button("Recompile shaders"))
        {
            myBasicShader.loadShader(
                "shaders/basic.vert",
                "shaders/basic.frag");
            initUniforms();
        }
        ImGui::End();
        processMovement();
      
   

  
	    renderScene();
        glCheckError();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

    delete ground_motion_state;
    dynamicsWorld->removeCollisionObject(ground_body);
    delete ground_body;
    delete ground_shape;
    for (const auto& object : objects)
    {
        GravityObject* grav = dynamic_cast<GravityObject*>(object.get());
        if (grav)
        {
            dynamicsWorld->removeCollisionObject(grav->getHitbox());
        }
    }

    delete dynamicsWorld;
    delete solver;
    delete overlappingPairCache;
    delete dispatcher;
    delete collisionConfiguration;

    return EXIT_SUCCESS;
}
