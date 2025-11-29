#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct Camera
{
    glm::vec3 Position = glm::vec3(0.0f, 0.0f, 0.0f);

    float NearClip = 0.1f;
    float FarClip = 100.0f;
    float Size = 10.0f;

    glm::mat4 ViewMatrix() const
    {
        return glm::lookAt(Position, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glm::mat4 ProjectionMatrix(float aspect) const
    {
        float h = Size / aspect / 2.0f;
        float w = Size / 2.0f;
        return glm::ortho(-w, w, -h, h, NearClip, FarClip);
    }
};
