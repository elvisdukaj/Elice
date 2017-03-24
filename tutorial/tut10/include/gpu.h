#pragma once

#include <iosfwd>
#include <string>
#include <GL/glew.h>
#include <vector>


enum class ShaderType {vertex, fragment};
class Shader {
public:

    Shader(ShaderType type, std::ifstream&& file);
    Shader(ShaderType type, const std::string& source);
    Shader(const Shader&) = delete;
    Shader(Shader&& rhs);

    Shader& operator = (const Shader&) = delete;
    Shader& operator = (Shader&& rhs);

    ~Shader();

private:
    friend class Program;

    operator GLuint() const noexcept
    {
        return m_shader;
    }

private:
    ShaderType m_type;
    GLuint m_shader;
};

class Program {
public:
    Program(std::vector<Shader>&& shaders);
    Program(const Program&) = delete;
    Program(Program&& rhs);

    Program& operator = (const Program&) = delete;
    Program& operator = (Program&& rhs);

    ~Program();

    void enable();
    void disable();

private:
    void link();
    void validate();

protected:
    GLuint m_program;

private:
    std::vector<Shader> m_shaders;
};
