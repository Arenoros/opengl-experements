#pragma once
#include "stdafx.h"
#include <string_view>
#include <vector>

namespace glsl {
#include <glsl/glsl.h>
    typedef std::basic_string_view<GLchar> shader_src;

#define MAP_SHADER(name) inline shader_src name((const GLchar*)name##_glsl, name##_glsl_len)

    MAP_SHADER(vertex);
    MAP_SHADER(fragm);
    MAP_SHADER(yelow);
    MAP_SHADER(uniform);
}  // namespace glsl

inline GLuint loadShaderFrom(std::string_view path, GLenum shaderType) {
    GLuint shaderID = 0;
    std::string shaderString;
    std::ifstream sourceFile(path.data());

    // Source file loaded
    if(sourceFile) {
        // Get shader source
        shaderString.assign((std::istreambuf_iterator<char>(sourceFile)), std::istreambuf_iterator<char>());

        // Set shader source
        const GLchar* shaderSource = shaderString.c_str();
        glShaderSource(shaderID, 1, (const GLchar**)&shaderSource, NULL);

        // Compile shader source
        glCompileShader(shaderID);

        // Check shader for errors
        GLint shaderCompiled = GL_FALSE;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &shaderCompiled);
        if(shaderCompiled != GL_TRUE) {
            printf("Unable to compile shader %d!\n\nSource:\n%s\n", shaderID, shaderSource);
            glDeleteShader(shaderID);
            shaderID = 0;
        }
    } else {
        printf("Unable to open file %s\n", path.data());
    }

    return shaderID;
}

class ShaderType {
    GLuint id;
    friend class ShaderProgram;

public:
    ShaderType(const ShaderType&) = delete;
    ShaderType& operator=(const ShaderType&) = delete;

    ShaderType(ShaderType&& rhs) noexcept: id(0) {
        if(&rhs == this) return;
        id = rhs.id;
        rhs.id = 0;
    }

    ShaderType& operator=(ShaderType&& rhs) noexcept {
        if(&rhs == this) return rhs;
        id = rhs.id;
        rhs.id = 0;
        return *this;
    }

    ShaderType(glsl::shader_src shader, GLenum shaderType) { id = create(shader, shaderType); }
    static GLuint create(const glsl::shader_src& shader, GLenum shaderType) {
        GLuint shaderID = glCreateShader(shaderType);
        // Set shader source
        const GLchar* shaderSource = shader.data();
        const GLint size = shader.size();
        glShaderSource(shaderID, 1, &shaderSource, &size);

        // Compile shader source
        glCompileShader(shaderID);

        // Check shader for errors
        GLint shaderCompiled = GL_FALSE;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &shaderCompiled);
        if(shaderCompiled != GL_TRUE) {
            GLchar infoLog[512];
            glGetShaderInfoLog(shaderID, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::" << shaderType << "::COMPILATION_FAILED\n" << infoLog << std::endl;
            glDeleteShader(shaderID);
            shaderID = 0;
        }
        return shaderID;
    }
    ~ShaderType() {
        if(!id) return;
        glDeleteShader(id);
        id = 0;
    }
};

class ShaderProgram {
    GLuint id;
    // std::vector<ShaderType> shaders;
public:
    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;
    ShaderProgram() { id = glCreateProgram(); }
    ShaderProgram(ShaderProgram&& rhs) noexcept: id(0) {
        if(&rhs == this) return;
        id = rhs.id;
        rhs.id = 0;
    }
    ShaderProgram& operator=(ShaderProgram&& rhs) noexcept {
        if(&rhs == this) return rhs;
        id = rhs.id;
        rhs.id = 0;
        return *this;
    }

    GLuint compile() {
        GLint success;
        glLinkProgram(id);
        glGetProgramiv(id, GL_LINK_STATUS, &success);
        if(!success) {
            GLchar infoLog[512];
            glGetProgramInfoLog(id, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        return id;
    }

    ~ShaderProgram() {
        if(!id) return;
        //...
        id = 0;
    }
    void attachShader(const ShaderType& shader) {
        glAttachShader(id, shader.id);
        // shaders.emplace_back(shader);
    }
    void run() { glUseProgram(id); }

    GLint uniformLoaction(const char* name) {
        return glGetUniformLocation(id, name);
    }
};
