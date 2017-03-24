#include "gpu.h"
#include <fstream>
#include <stdexcept>

using namespace std;

namespace detail {

GLenum toShaderType(ShaderType t)
{
    switch (t)
    {
    case ShaderType::vertex:
        return GL_VERTEX_SHADER;

    case ShaderType::fragment:
        return GL_FRAGMENT_SHADER;
    };

    return GL_VERTEX_SHADER;
}

GLuint CreateShader(ShaderType type, const string& source)
{
    GLuint shaderObj = glCreateShader(toShaderType(type));

    if (!shaderObj)
        throw runtime_error{"Unable to create the shader object"};

    const GLchar* sources[1] = { source.c_str() };
    GLint lengths[1] = {GLint(source.length())};
    glShaderSource(shaderObj, 1, sources, lengths);
    glCompileShader(shaderObj);

    GLint buildResult;
    glGetShaderiv(shaderObj, GL_COMPILE_STATUS, &buildResult);

    if (!buildResult)
    {
        GLint logLength;
        glGetShaderiv(shaderObj, GL_INFO_LOG_LENGTH, &logLength);

        string logMessage(logLength, ' ');
        glGetShaderInfoLog(shaderObj, logLength, nullptr, &logMessage[0]);

        throw runtime_error{logMessage};
    }

    return shaderObj;
}

GLuint CreateShader(ShaderType type, ifstream& file)
{
    if (!file)
        throw invalid_argument{"The specified file doesn't exists"};

    string source;

    file.seekg(0, ios::end);
    source.reserve(file.tellg());
    file.seekg(0, ios::beg);

    source.assign(istreambuf_iterator<char>{file},
                  istreambuf_iterator<char>{ });

    return CreateShader(type, source);
}

} // detail

Shader::Shader(ShaderType type, ifstream&& file)
    : m_type{type}, m_shader{detail::CreateShader(type, file)}
{
}

Shader::Shader(ShaderType type, const string& file)
    : m_type{type}, m_shader{detail::CreateShader(type, file)}
{
}

Shader::Shader(Shader&& rhs)
    : m_type{rhs.m_type}, m_shader{rhs.m_shader}
{
    rhs.m_shader = 0;
}

Shader& Shader::operator = (Shader&& rhs)
{
    m_type = rhs.m_type;
    m_shader = rhs.m_shader;
    rhs.m_shader = 0;

    return *this;
}

Shader::~Shader()
{
    if (m_shader != 0)
        glDeleteShader(m_shader);
}

Program::Program(vector<Shader>&& shaders)
    : m_program{glCreateProgram()}
    , m_shaders{move(shaders)}
{
    if (!m_program)
        throw runtime_error{"Unable to create gpu program"};

    for (const auto& s : m_shaders)
        glAttachShader(m_program, static_cast<GLuint>(s));

    link();
    validate();

    if (glGetError() != GL_NO_ERROR)
        throw runtime_error{"Unable to get scale_uniform location"};
}

Program::Program(Program&& rhs)
    : m_program(rhs.m_program)
    , m_shaders(move(rhs.m_shaders))
{
    rhs.m_program = 0;
}

Program& Program::operator = (Program&& rhs)
{
    m_program = rhs.m_program;
    m_shaders = move(rhs.m_shaders);
    rhs.m_program = 0;
    return *this;
}

Program::~Program()
{
    if (m_program)
    {
        for(const auto& s : m_shaders)
            glDetachShader(m_program, static_cast<GLuint>(s));

        glDeleteProgram(m_program);
    }
}

void Program::disable()
{
    glUseProgram(0);
}

void Program::enable()
{
    glUseProgram(m_program);
}

void Program::link()
{
    GLint linkResult;
    glLinkProgram(m_program);
    glGetProgramiv(m_program, GL_LINK_STATUS, &linkResult);
    if (!linkResult)
    {
        GLint logLength;
        glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &logLength);

        string logMessage(logLength, ' ');
        glGetProgramInfoLog(m_program, logLength, nullptr, &logMessage[0]);

        throw runtime_error{logMessage};
    }
}

void Program::validate()
{
    GLint validateResult;
    glGetProgramiv(m_program, GL_VALIDATE_STATUS, &validateResult);
    if (!validateResult)
    {
        GLint logLength;
        glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &logLength);

        string logMessage(logLength, ' ');
        glGetProgramInfoLog(m_program, logLength, nullptr, &logMessage[0]);

        throw runtime_error{logMessage};
    }
}
