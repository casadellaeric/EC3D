#pragma once

#include <glm/mat4x4.hpp>

namespace ec
{

struct PerspectiveCameraProperties {
    float aspectRatio;
    float fovY;
    float zNear;
    float zFar;
};

struct OrthographicCameraProperties {
    float halfWidth;
    float halfHeight;
    float zNear;
    float zFar;
};

class Camera
{
public:

    enum class Type {
        perspective,
        orthographic,
    };

    Camera() = default;
    Camera(PerspectiveCameraProperties&& perspectiveProperties) :
      m_cameraType(Camera::Type::perspective),
      m_perspectiveProperties(perspectiveProperties){};
    Camera(OrthographicCameraProperties&& orthographicProperties) :
      m_cameraType(Camera::Type::orthographic),
      m_orthographicProperties(orthographicProperties){};

    // TODO: Create view/projection matrices from constructor

private:

    std::string m_name{};

    Camera::Type m_cameraType{};

    std::optional<PerspectiveCameraProperties> m_perspectiveProperties{};
    std::optional<OrthographicCameraProperties> m_orthographicProperties{};

    glm::mat4 m_viewMatrix{ 1.f };
    glm::mat4 m_projectionMatrix{ 1.f };
};

}  // namespace ec
