#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Pipeline {
public:
    Pipeline& scale(const glm::vec3& scale) noexcept
    {
        m_transformation = glm::scale(m_transformation, scale);
        return *this;
    }

    Pipeline& worldPos(const glm::vec3& pos) noexcept
    {
        m_transformation = glm::translate(m_transformation, pos);
        return *this;
    }

    Pipeline& rotate(float angle, const glm::vec3& axies) noexcept
    {
        m_transformation = glm::rotate(m_transformation, angle, axies);
        return *this;
    }

    operator const glm::mat4& () const noexcept
    {
        return m_transformation;
    }

private:
    glm::mat4 m_transformation;
};
