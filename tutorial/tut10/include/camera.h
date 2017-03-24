#pragma once

#include <glm/glm.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    Camera()
        : m_pos(0.0f, 0.0f, -1.0f)
        , m_target(0.0f, 0.0f, 1.0f)
        , m_up(0.0f, 1.0f, 0.0f)
        , m_projection{glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f)}
    {
    }

    Camera& ortho(float left, float right, float bottom, float up, float near, float far) noexcept
    {
        m_projection = glm::ortho(left, right, bottom, up, near, far);
    }

    Camera& perspective(float fov, float ratio, float near, float far) noexcept
    {
        m_projection = glm::perspective(fov, ratio, near, far);
    }

    Camera& position(const glm::vec3& newPos) noexcept
    {
        m_pos = newPos;
        return *this;
    }

    Camera& target(const glm::vec3& newTarget) noexcept
    {
        m_target = newTarget;
        glm::normalize(m_up);
        return *this;
    }

    Camera& up(const glm::vec3& newUp) noexcept
    {
        m_up = newUp;
        glm::normalize(m_up);
        return *this;
    }

    const glm::vec3& position() const noexcept
    {
        return m_pos;
    }

    const glm::vec3& target() const noexcept
    {
        return m_target;
    }

    const glm::vec3& up() const noexcept
    {
        return m_up;
    }

    Camera& offsetPosition(const glm::vec3& offset) noexcept
    {
        m_pos += offset;
        return *this;
    }

    Camera& offsetTarget(const glm::vec3& offset) noexcept
    {
        m_target += offset;
        glm::normalize(m_up);
        return *this;
    }

    Camera& offsetUp(const glm::vec3& offset) noexcept
    {
        m_up += offset;
        glm::normalize(m_up);
        return *this;
    }

    Camera& forward(float delta) noexcept
    {
        m_pos.z += delta;
        return *this;
    }

    Camera& lateral(float delta) noexcept
    {
        m_pos += glm::cross(m_up, m_target) * delta;
        return *this;
    }

    operator glm::mat4 () const noexcept
    {
        return m_projection * glm::lookAt(m_pos, m_pos + m_target, m_up);
    }

private:
    glm::vec3 m_pos;
    glm::vec3 m_target;
    glm::vec3 m_up;
    glm::mat4 m_projection;
};
