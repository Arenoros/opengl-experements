#pragma once
#include "stdafx.h"

#define __NEW_GL_SHADER(Name)                                                                                          \
    template<GLenum _Type>                                                                                             \
    struct Name {                                                                                                      \
        static const shader_src src;                                                                                   \
        static const bool exist;                                                                                       \
    };                                                                                                                 \
    template<GLenum _Type>                                                                                        \
    const bool Name<_Type>::exist = false;

#define ENABLE_SHADER_TYPE(Name, type)                                                                                 \
    template<>                                                                                                         \
    const bool Name<type>::exist = true;

#define DEF_SHADER(Name)                                                                                               \
    __NEW_GL_SHADER(Name)                                                                                              \
    ENABLE_SHADER_TYPE(Name, GL_VERTEX_SHADER);                                                                        \
    ENABLE_SHADER_TYPE(Name, GL_FRAGMENT_SHADER)

namespace glsl {
    enum shader_type { vertex = GL_VERTEX_SHADER, fragment = GL_FRAGMENT_SHADER, custom };
    typedef std::basic_string_view<GLchar> shader_src;
}  // namespace glsl

namespace glsl {
    DEF_SHADER(shader);

    struct ShaderRes {
        GLuint id;
        friend class ShaderProgram;

    public:
        ShaderRes(const ShaderRes&) = delete;
        ShaderRes& operator=(const ShaderRes&) = delete;

        ShaderRes(ShaderRes&& rhs) noexcept: id(0) {
            if(&rhs == this) return;
            id = rhs.id;
            rhs.id = 0;
        }

        ShaderRes& operator=(ShaderRes&& rhs) noexcept {
            if(&rhs == this) return rhs;
            id = rhs.id;
            rhs.id = 0;
            return *this;
        }

        ShaderRes(glsl::shader_src shader, GLenum shaderType) { id = create(shader, shaderType); }
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
        ~ShaderRes() {
            if(!id) return;
            glDeleteShader(id);
            id = 0;
        }
    };

    template<template<GLenum> class _Shader>
    struct Program {
        GLuint id;
        Program() {
            id = glCreateProgram();
            if constexpr(_Shader<vertex>::exist) loadShader<vertex>();
            if constexpr(_Shader<fragment>::exist) loadShader<fragment>();
            compile();
        }
        Program(const Program&) = delete;
        Program& operator=(const Program&) = delete;
        Program(Program&& rhs) noexcept: id(0) {
            if(&rhs == this) return;
            id = rhs.id;
            rhs.id = 0;
        }
        Program& operator=(Program&& rhs) noexcept {
            if(&rhs == this) return rhs;
            id = rhs.id;
            rhs.id = 0;
            return *this;
        }
        void set(const char* name, GLfloat val) {
            GLint uniform = getUniform(name);
            glUniform1f(uniform, val);
        }
        void set(const char* name, GLfloat val[3]) {
            GLint uniform = getUniform(name);
            glUniform3fv(uniform, 1, &val[0]);
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

        ~Program() {
            if(!id) return;
            //...
            id = 0;
        }
        template<shader_type _Type>
        void loadShader() {
            ShaderRes res(_Shader<_Type>::src, _Type);
            attachShader(res);
        }
        void attachShader(const ShaderRes& shader) const {
            glAttachShader(id, shader.id);
            // shaders.emplace_back(shader);
        }
        void use() { glUseProgram(id); }

        GLint getUniform(const char* name) const { return glGetUniformLocation(id, name); }
    };

}  // namespace glsl

inline GLuint loadShaderFrom(std::string_view path, GLenum shaderType) {
    GLuint shaderID = 0;
    std::string shaderString;
    std::ifstream sourceFile(path.data());
    // Source file loaded
    if(sourceFile) {
        // Get shader source
        shaderString.assign((std::istreambuf_iterator<char>(sourceFile)), std::istreambuf_iterator<char>());
        shaderID = glCreateShader(shaderType);
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
#undef __NEW_GL_SHADER
#undef ENABLE_SHADER_TYPE
#undef DEF_SHADER

