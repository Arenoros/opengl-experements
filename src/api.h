#pragma once
#include <array>
#include <cassert>
#include <functional>
#include "glsl_loader.h"

inline void baseInit() {
    //Инициализация GLFW
    glfwInit();
    //Настройка GLFW
    //Задается минимальная требуемая версия OpenGL.
    //Мажорная
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    //Минорная
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_ALPHA_BITS, GL_TRUE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    //Выключение возможности изменения размера окна
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
}

inline GLFWwindow* createMainWindow(int width, int height, const char* title = nullptr) {
    GLFWwindow* window = glfwCreateWindow(width, height, title ? title : "", nullptr, nullptr);
    if(window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    int _width, _height;
    glfwGetFramebufferSize(window, &_width, &_height);

    glViewport(0, 0, _width, _height);
    return window;
}

template<size_t N>
struct IndexBuffer {
    GLuint buf;
    GLuint data[N];
    IndexBuffer(const IndexBuffer&) = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;
    ~IndexBuffer() { glDeleteBuffers(1, &buf); }
    /*VertexBuffer(float&&... data): vertices({std::move(data)...}) {
        glGenBuffers(1, &VBO);
    }*/
    IndexBuffer(GLuint data[N * 3]) : data(data) { glGenBuffers(1, &buf); }
    IndexBuffer(std::initializer_list<GLuint> l) {
        glGenBuffers(1, &buf);
        assert(l.size() == N);
        std::copy(l.begin(), l.begin() + N, data);
    }

    IndexBuffer& operator=(std::initializer_list<GLuint> l) {
        assert(l.size() == N * 3);
        std::copy(l.begin(), l.begin() + N, data);
        return *this;
    }

    void bind(GLenum target, GLenum usage) {
        glBindBuffer(target, buf);
        glBufferData(target, sizeof(data), data, usage);
    }
};

template<class... U>
IndexBuffer(GLuint, U...)->IndexBuffer<1 + sizeof...(U)>;

template<size_t Size>
IndexBuffer(GLuint(&)[Size])->IndexBuffer<Size>;

template<size_t N>
struct VertexBuffer {
    GLuint buf;
    GLfloat data[N];
    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer<N>& lv) {
        glGenBuffers(1, &buf);
        std::memcpy(data, lv.data, sizeof(data));
        return *this;
    }
    ~VertexBuffer() { glDeleteBuffers(1, &buf); }
    /*VertexBuffer(float&&... data): vertices({std::move(data)...}) {
        glGenBuffers(1, &VBO);
    }*/
    VertexBuffer(GLfloat data[N]) : data(data) { glGenBuffers(1, &buf); }
    VertexBuffer(std::initializer_list<GLfloat> l) {
        glGenBuffers(1, &buf);
        assert(l.size() == N);
        std::copy(l.begin(), l.begin() + N, data);
    }

    VertexBuffer& operator=(std::initializer_list<GLfloat> l) {
        assert(l.size() == N);
        std::copy(l.begin(), l.begin() + N, data);
        return *this;
    }

    void bind(GLenum target, GLenum usage) {
        glBindBuffer(target, buf);
        glBufferData(target, sizeof(data), data, usage);
    }
};
template<class... U>
VertexBuffer(GLfloat, U...)->VertexBuffer<1 + sizeof...(U)>;

template<size_t Size>
VertexBuffer(GLfloat(&)[Size])->VertexBuffer<Size>;

template<size_t N>
class VertexArrays {
    GLuint VAOs[N];

public:
    VertexArrays() { glGenVertexArrays(N, VAOs); }
    ~VertexArrays() { glDeleteVertexArrays(N, VAOs); }
    template<size_t Pos>
    void on(std::function<void()> fun) {
        static_assert(Pos < N);
        // 1. Привязываем VAO
        glBindVertexArray(VAOs[Pos]);
        fun();
        // 4. Отвязываем VAO
        glBindVertexArray(0);
    }
    void on(int Pos, std::function<void()> fun) {
        // 1. Привязываем VAO
        glBindVertexArray(VAOs[Pos]);
        fun();
        // 4. Отвязываем VAO
        glBindVertexArray(0);
    }
    void bind(int Pos) {
        assert(Pos < N);
        // 1. Привязываем VAO
        glBindVertexArray(VAOs[Pos]);
    }
    void unbind() {
        glBindVertexArray(0);
    }
};
template<class T>
struct GLType;

template<> struct GLType<GLfloat> { enum { type = GL_FLOAT }; };
template<> struct GLType<GLint> { enum { type = GL_INT }; };

template<class T>
void setVertexAttribute(GLint pos, GLint count, GLboolean normalize, GLint step, GLint off) {
    glVertexAttribPointer(pos, count, GLType<T>::type, normalize, step * sizeof(T), (GLvoid*)(off * sizeof(T)));
    glEnableVertexAttribArray(pos);
}



class File {
    FILE* ptr;
    std::string path;

public:
    File(std::string_view path, std::string_view mode) : path(path) { ptr = fopen(path.data(), mode.data()); }
    ~File() {
        if(ptr) fclose(ptr);
    }
    template<class T>
    int read(T* buffer, int size) {
        if(!ptr) return 0;
        return fread(buffer, sizeof(T), size/ sizeof(T), ptr);
    }
    template<class T>
    int write(T* buffer, int size) {
        if(!ptr) return 0;
        return fwrite(buffer, sizeof(T), size/ sizeof(T), ptr);
    }
    operator bool() const { return ptr != nullptr; }
    int size() const {
        if(!ptr) return -1;
        struct _stat st;
        _stat(path.c_str(), &st);
        return st.st_size;
    }
};
