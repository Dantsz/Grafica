    cmake_minimum_required (VERSION 3.8)
 
    set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)
    set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "3c375311-a3c9-4396-a187-3227ef642046")
    add_executable (CMakeProject1 
    "main.cpp" 
    "GPSLab1.cpp""GPSLab1.hpp" 
    "Shader.cpp""Shader.hpp" 
    "Camera.cpp"  
    "Mesh.cpp" 
    "Model3D.cpp"
    "Window.cpp""Window.h" 
    "Object.h" "Object.cpp"
    "SkyBox.hpp""SkyBox.cpp"
    )
    target_sources(CMakeProject1 PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES GravityObject.ixx)
find_package(glm CONFIG REQUIRED)
    target_link_libraries(CMakeProject1 PRIVATE glm::glm)
find_package(GLEW REQUIRED)
    target_link_libraries(CMakeProject1 PRIVATE GLEW::GLEW)
find_package(glfw3 CONFIG REQUIRED)
    target_link_libraries(CMakeProject1 PRIVATE glfw)
find_package(imgui CONFIG REQUIRED)
    target_link_libraries(CMakeProject1 PRIVATE imgui::imgui)
find_path(STB_INCLUDE_DIRS "stb_c_lexer.h")   
    target_include_directories(CMakeProject1 PRIVATE ${STB_INCLUDE_DIRS})
find_package(tinyobjloader CONFIG REQUIRED)
    target_link_libraries(CMakeProject1 PRIVATE tinyobjloader::tinyobjloader)
find_package(Bullet CONFIG REQUIRED)
    target_link_libraries(CMakeProject1 PRIVATE ${BULLET_LIBRARIES})
if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET CMakeProject1 PROPERTY CXX_STANDARD 20)
endif()