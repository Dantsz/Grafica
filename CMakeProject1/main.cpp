
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


// light parameters
glm::vec3 lightDir = {0.0f, 1.0f, 1.0f};

glm::vec3 lightColor = glm::vec3(1.0f, 1.0f,1.0f); //white light




// camera
gps::Camera myCamera(glm::vec3(0.0f, 5.0f, 15.0f), glm::vec3(0.0f, 2.0f, -10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
constexpr float render_distance = 2000.0f;
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
float lightAngle = 0.0f;

glm::mat4 lightMatrixTR;
glm::mat3 normalMatrix;
//generate FBO ID


const unsigned int SHADOW_WIDTH = 4196;
const unsigned int SHADOW_HEIGHT = 4196;


std::vector<std::unique_ptr<Object>> objects{};
//skybox
std::vector<const GLchar*> faces;
gps::SkyBox mySkyBox;
gps::Shader skyboxShader;
//point light
float constant = 1.f;
float linear = 0.09f;
float quadratic = 0.032f;

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
float far_plane = 1000.0f;
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

    if (pressedKeys[GLFW_KEY_Q]) {
        objects[0]->rotate(-1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    if (pressedKeys[GLFW_KEY_E]) {
        objects[0]->rotate(1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
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
    glEnable(GL_FRAMEBUFFER_SRGB);
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
	myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");
    depthMapShader.loadShader ("shaders/shadow.vert", "shaders/shadow.frag","shaders/shadow.geo");
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

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    skyboxShader.useShaderProgram();
    view = myCamera.getViewMatrix();
    projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
    mySkyBox.Draw(skyboxShader, view, projection);

    //NORMAL
    myBasicShader.useShaderProgram();

    view = myCamera.getViewMatrix();
    myBasicShader.setMat4("view", view);

    const auto lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    static glm::vec3 lightVec{};
    lightVec = glm::inverseTranspose(glm::mat3(lightRotation)) * lightDir;
    myBasicShader.setVec3("lightDir", lightVec);
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

    
    objects.emplace_back(std::make_unique<GravityObject>(teapot_model));
    objects.emplace_back(std::make_unique<Object>(teapot_model));
    objects.emplace_back(std::make_unique<Object>(sponza_model));

    objects[objects.size() - 1]->set_scale({ 0.1f,0.1f,0.1f });


    btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
    btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
    btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();
    btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();
    btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,
        overlappingPairCache, solver, collisionConfiguration);

   

    
    dynamicsWorld -> setGravity(btVector3(0, -10, 0));
    btAlignedObjectArray<btCollisionShape*> collisionShapes;
    {
       // btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(50.), btScalar(50.), btScalar(50.)));
        btVector3 planeNormal = btVector3(0.0f, 1.0f, 0.0f);
        btStaticPlaneShape* groundShape = new btStaticPlaneShape(planeNormal, 0);
        collisionShapes.push_back(groundShape);

        btTransform groundTransform;
        groundTransform.setIdentity();
        groundTransform.setOrigin(btVector3(0, 0, 0));

        btScalar mass(0.);

        //rigidbody is dynamic if and only if mass is non zero, otherwise static
        bool isDynamic = (mass != 0.f);

        btVector3 localInertia(0, 0, 0);
        if (isDynamic)
            groundShape->calculateLocalInertia(mass, localInertia);

        //using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
        btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
        btRigidBody* body = new btRigidBody(rbInfo);

        //add the body to the dynamics world
        dynamicsWorld->addRigidBody(body);
    }
    btRigidBody* teapot_body = nullptr;
    {
        //create a dynamic rigidbody

        //btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
        btCollisionShape* colShape = new btSphereShape(btScalar(.25));
        collisionShapes.push_back(colShape);

        /// Create Dynamic Objects
        btTransform startTransform;
        startTransform.setIdentity();

        btScalar mass(1.f);

        //rigidbody is dynamic if and only if mass is non zero, otherwise static
        bool isDynamic = (mass != 0.f);

        btVector3 localInertia(0, 0, 0);
        if (isDynamic)
            colShape->calculateLocalInertia(mass, localInertia);

        startTransform.setOrigin(btVector3(2, 10, 0));

        //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
        btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
        teapot_body = new btRigidBody(rbInfo);

        dynamicsWorld->addRigidBody(teapot_body);
    }
    for (auto i = 0; i < 100; i++)
         {
        
        
             // print positions of all objects
             for (int j = dynamicsWorld -> getNumCollisionObjects() - 1; j >= 0; j--)
             {
                 btCollisionObject * obj = dynamicsWorld -> getCollisionObjectArray()[j];
                 btRigidBody * body = btRigidBody::upcast(obj);
                 btTransform trans;
                 if (body && body -> getMotionState())
                     {
                     body->getMotionState()->getWorldTransform(trans);
                
                     }
                 else
                     {
                     trans = obj -> getWorldTransform();
                     }
                 printf(" world pos object %d = %f ,%f ,%f\n", j, float(trans.getOrigin().getX()), float(
                    trans.getOrigin().getY()), float(trans.getOrigin().getZ()));
                 }
         }

	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        dynamicsWorld->stepSimulation(1.f / 60.f, 10);
        btTransform trans;
        teapot_body->getMotionState()->getWorldTransform(trans);
        printf(" world pos object  = %f ,%f ,%f\n", float(trans.getOrigin().getX()), float(
            trans.getOrigin().getY()), float(trans.getOrigin().getZ()));

        objects[0]->setPosition(glm::vec3(float(trans.getOrigin().getX()), float(
            trans.getOrigin().getY()), float(trans.getOrigin().getZ())));
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
            ImGui::DragFloat3("Direction", glm::value_ptr(lightDir),0.1,-1.0,1.0);
            ImGui::InputFloat("Angle", &lightAngle, 1.f);
            ImGui::InputFloat3("LightPos", glm::value_ptr(lightPos));
            ImGui::InputFloat("Near plane", &near_plane, 0.1f, 1.0f);
            ImGui::InputFloat("Far Plane", &far_plane, 0.1f, 1.0f);
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
    delete dynamicsWorld;
    delete solver;
    delete overlappingPairCache;
    delete dispatcher;
    delete collisionConfiguration;

    return EXIT_SUCCESS;
}
