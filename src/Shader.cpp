//
// Created by Fedor Artemenkov on 05.07.2024.
//

#include "Shader.h"
#include <glad/glad.h>
#include <cstdio>

#include <iostream>
#include <fstream>
#include <sstream>

Shader::Shader()
{
    program = 0;
}

Shader::Shader(Shader &&other) noexcept
{
    program = other.program;
    other.program = 0;
}

Shader &Shader::operator=(Shader &&other) noexcept
{
    program = other.program;
    other.program = 0;

    return *this;
}

Shader::~Shader()
{
    glDeleteShader(program);
}

unsigned int compile_shader(unsigned int type, const char* source);

void Shader::init(const char *vert, const char *frag)
{
    program = glCreateProgram();

    unsigned int vs = compile_shader(GL_VERTEX_SHADER, vert);
    unsigned int fs = compile_shader(GL_FRAGMENT_SHADER, frag);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);
}

void Shader::bind() const
{
    glUseProgram(program);
}

void Shader::unbind() const
{
    glUseProgram(0);
}



//void Shader::setUniformMatrix(const float *data, const char *name) const
//{
//    const GLint location = glGetUniformLocation(program, name);
//
//    if (location == -1)
//    {
//        printf("Shader have no uniform %s\n", name);
//        return;
//    }
//
//    glUniformMatrix4fv(location, 1, GL_FALSE, data);
//}
//
//void Shader::setUniformMatrices(const float *data, int num, const char *name) const
//{
//    const GLint location = glGetUniformLocation(program, name);
//
//    if (location == -1)
//    {
//        printf("Shader have no uniform %s\n", name);
//        return;
//    }
//
//    glUniformMatrix4fv(location, num, GL_FALSE, data);
//}

//void Shader::setUniformVector4(const float *data, const char *name) const
//{
//    const GLint location = glGetUniformLocation(program, name);
//
//    if (location == -1)
//    {
//        printf("Shader have no uniform %s\n", name);
//        return;
//    }
//
//    glUniform4fv(location, 1, data);
//}

void Shader::init(const char *filepath)
{
    enum ShaderSourceType {
        SRC_NONE = -1,
        SRC_VERTEX = 0,
        SCR_FRAGMENT = 1
    };

    std::ifstream stream(filepath);
    
    if (!stream.is_open())
    {
        std::cerr << "Error: Failed to open the file at path: " << filepath << std::endl;
        return;
    }

    std::string line;
    std::stringstream vertexStream;
    std::stringstream fragmentStream;

    ShaderSourceType currentType = SRC_NONE;

    while (getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
            {
                currentType = SRC_VERTEX;

            }
            else if (line.find("fragment") != std::string::npos)
            {
                currentType = SCR_FRAGMENT;
            }
        }
        else if (currentType == SRC_VERTEX)
        {
            vertexStream << line << "\n";
        }
        else if (currentType == SCR_FRAGMENT)
        {
            fragmentStream << line << "\n";
        }
    }

    init(vertexStream.str().c_str(), fragmentStream.str().c_str());
}

void Shader::setUniform(const std::string& name, const glm::vec3& vector) const
{
    const GLint location = glGetUniformLocation(program, name.c_str());

    if (location == -1)
    {
        printf("Shader have no uniform %s\n", name.c_str());
        return;
    }

    glUniform3fv(location, 1, (const float*) &vector);
}

void Shader::setUniform(const std::string& name, const glm::vec4& vector) const
{
    const GLint location = glGetUniformLocation(program, name.c_str());

    if (location == -1)
    {
        printf("Shader have no uniform %s\n", name.c_str());
        return;
    }

    glUniform4fv(location, 1, (const float*) &vector);
}

void Shader::setUniform(const std::string &name, const glm::mat4& matrix) const
{
    const GLint location = glGetUniformLocation(program, name.c_str());

    if (location == -1)
    {
        printf("Shader have no uniform %s\n", name.c_str());
        return;
    }
    
    glUniformMatrix4fv(location, 1, GL_FALSE, (const float*) &matrix);
}

void Shader::setUniform(const std::string &name, const std::vector<glm::vec3> &vectors) const
{
    const GLint location = glGetUniformLocation(program, name.c_str());

    if (location == -1)
    {
        printf("Shader have no uniform %s\n", name.c_str());
        return;
    }
    
    glUniform3fv(location, (GLsizei)(vectors.size()), &(vectors[0][0]));
}

void Shader::setUniform(const std::string &name, const std::vector<glm::mat4> &matrices) const
{
    const GLint location = glGetUniformLocation(program, name.c_str());

    if (location == -1)
    {
        printf("Shader have no uniform %s\n", name.c_str());
        return;
    }
    
    glUniformMatrix4fv(location, (GLsizei)(matrices.size()), GL_FALSE, &(matrices[0][0][0]));
}


unsigned int compile_shader(unsigned int type, const char* source)
{
    unsigned int id = glCreateShader(type);

    glShaderSource(id, 1, &source, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);

    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);

        char message[1024];
        glGetShaderInfoLog(id, length, &length, message);

        printf("Failed to compile %s shader:\n", (type == GL_VERTEX_SHADER) ? "vertex" : "fragment");
        printf("%s\n", message);
        return 0;
    }

    return id;
}
