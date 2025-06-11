//
// Created by Fedor Artemenkov on 05.07.2024.
//

#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>

class Shader
{
public:
    Shader();

    Shader(const Shader&) = delete;             // no copy allowed
    Shader& operator =(const Shader&) = delete; // no copy allowed

    Shader(Shader&& other) noexcept;            // move allowed
    Shader& operator=(Shader&& other) noexcept; // move allowed

    ~Shader();

public:
    void init(const char* filename);
    void init(const char* vert_src, const char* frag_src);

    void bind() const;
    void unbind() const;

    void setUniform(const std::string& name, const glm::vec3& vector) const;
    void setUniform(const std::string& name, const glm::vec4& vector) const;
    void setUniform(const std::string& name, const glm::mat4& matrix) const;
    
    void setUniform(const std::string& name, const std::vector<glm::vec3>& vectors) const;
    void setUniform(const std::string& name, const std::vector<glm::mat4>& matrices) const;

//private:
    int program;
};
