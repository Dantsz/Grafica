//
//  GPSLab1.cpp
//
//  Copyright © 2017 CGIS. All rights reserved.
//

#include "GPSLab1.hpp"

namespace gps {
    glm::vec4 TransformPoint(const glm::vec4 &point)
    {
        return glm::vec4(1.0f);
    }
    
    float ComputeAngle(const glm::vec3 &v1, const glm::vec3 &v2)
    {
        glm::vec3 da = glm::normalize(v1);
        glm::vec3 db = glm::normalize(v2);
        return glm::degrees(glm::acos(glm::dot(da, db)));
    }
    
    bool IsConvex(const std::vector<glm::vec2> &vertices)
    {
        if (vertices.size() < 3)
        {
            return false;
        }
        for (size_t i = 1; i < vertices.size(); i++)
        {
            if (glm::sign(glm::dot(vertices[i], vertices[(i + 1) % vertices.size()])) != glm::sign(glm::dot(vertices[i - 1], vertices[(i) % vertices.size()])))
            {
                return false;
            }
        }
        return true;
    }
    
    std::vector<glm::vec2> ComputeNormals(const std::vector<glm::vec2> &vertices)
    {
        std::vector<glm::vec2> normalsList{};
        for (size_t i = 1 ; i < vertices.size() ; i++)
        {
            normalsList.push_back(glm::cross(vertices[i - 1], vertices[i]));
        }
        return normalsList;
    }
}
