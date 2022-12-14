    cmake_minimum_required (VERSION 3.8)
    add_executable (CMakeProject1 
    "main.cpp" 
    "GPSLab1.cpp""GPSLab1.hpp" 
    "Shader.cpp""Shader.hpp" 
    "Camera.cpp"  
    "Mesh.cpp" 
    "Model3D.cpp"
    "Window.cpp""Window.h" 
    "Object.h" "Object.cpp"
   )
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

if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET CMakeProject1 PROPERTY CXX_STANDARD 20)
endif()