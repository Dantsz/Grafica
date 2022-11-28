#include "Camera.hpp"
#include <glm/gtx/euler_angles.hpp>

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) :
        cameraPosition(cameraPosition),
        cameraTarget(cameraTarget),
        cameraUpDirection(cameraUp),
        cameraFrontDirection(glm::normalize(cameraTarget - cameraPosition)),
        cameraRightDirection(glm::rotate(glm::radians(360.0f - 90.0f), glm::vec3(0.0f, 1.0f, 0.0f))* glm::vec4(glm::normalize(cameraTarget - cameraPosition), 0.0f))

    {
  
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
  

        return glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection);
        // return glm::lookAt(cameraPosition, cameraPosition + cameraFrontDirection, cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {

    
        glm::vec3 movement{ 0.0f };
        float angle = 0.0f;
        switch (direction)
        {
        case MOVE_FORWARD:
            movement += speed * cameraFrontDirection;
            break;
        case MOVE_BACKWARD:
            movement -= speed * cameraFrontDirection;
            break;
        case MOVE_RIGHT:
            movement += speed * cameraRightDirection;
            break;
        case MOVE_LEFT:
            movement -= speed * cameraRightDirection;
            break;
        }
        cameraTarget += movement;
        cameraPosition = cameraPosition + movement;

    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        /*  auto cameraPos = glm::rotate(glm::mat4(1.0f), pitch, cameraRightDirection);
          cameraPos = glm::rotate(cameraPos, yaw, cameraUpDirection);
          cameraTarget = glm::vec4(cameraTarget,0.0f) * cameraPos;
          cameraFrontDirection = glm::vec4(cameraFrontDirection,.0f) * cameraPos;
          cameraRightDirection = glm::vec4(cameraRightDirection, .0f) * cameraPos;*/

        auto cameraPos = glm::rotate(glm::mat4(1.0f), yaw, cameraUpDirection);
        cameraPos = glm::rotate(cameraPos, pitch, cameraRightDirection);

        cameraTarget = glm::vec4(cameraTarget, 0.0f) * cameraPos;
        cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        cameraRightDirection = glm::rotate(glm::radians(360.0f - 90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(glm::normalize(cameraTarget - cameraPosition), 0.0f);
    }
}