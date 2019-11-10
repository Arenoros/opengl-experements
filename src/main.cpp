#include "stdafx.h"
#include "glsl_loader.h"
#include <array>
#include <cassert>
#include <functional>

void baseInit() {
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

GLFWwindow* createMainWindow(int width, int height, const char* title = nullptr) {
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
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    // Когда пользователь нажимает ESC, мы устанавливаем свойство WindowShouldClose в true,
    // и приложение после этого закроется
    if(action != GLFW_PRESS) return;
    switch(key) {
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GL_TRUE);
        break;
    case GLFW_KEY_F:
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // не работает
        break;
    case GLFW_KEY_L:
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  // не работает
        break;
    }
}

void drop_callback(GLFWwindow* window, int count, const char** paths) {
    for(int i = 0; i < count; i++) std::cout << paths[i];
}

template<size_t N>
struct IndexBuffer {
    GLuint buf;
    GLuint data[N];
    IndexBuffer(const IndexBuffer&) = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;
    ~IndexBuffer() {}
    /*VertexBuffer(float&&... data): vertices({std::move(data)...}) {
        glGenBuffers(1, &VBO);
    }*/
    IndexBuffer(GLuint data[N * 3]): data(data) { glGenBuffers(1, &buf); }
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
IndexBuffer(GLuint (&)[Size])->IndexBuffer<Size>;

template<size_t N>
struct VertexBuffer {
    GLuint buf;
    GLfloat data[N];
    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;
    ~VertexBuffer() {}
    /*VertexBuffer(float&&... data): vertices({std::move(data)...}) {
        glGenBuffers(1, &VBO);
    }*/
    VertexBuffer(GLfloat data[N]): data(data) { glGenBuffers(1, &buf); }
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
VertexBuffer(GLfloat (&)[Size])->VertexBuffer<Size>;

template<size_t N>
class VertexArrays {
    GLuint VAOs[N];

public:
    VertexArrays() { glGenVertexArrays(N, VAOs); }
    template<size_t Pos>
    void on(std::function<void()> fun) {
        static_assert(Pos < N);
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
};
template<class T>
struct GLType;
#define GLTypeMap(CType, _GLType)                                                                                       \
    template<>                                                                                                         \
    struct GLType<CType> {                                                                                             \
        enum { type = _GLType };                                                                                        \
    }

GLTypeMap(GLfloat, GL_FLOAT);
GLTypeMap(GLint, GL_INT);


template<class T>
void setAttribute(GLint pos, GLint count, GLboolean normalize, GLint step, GLint off) {
    off *= sizeof(T);
    glVertexAttribPointer(pos, count, GLType<T>::type, normalize, step * sizeof(T), &off);
    glEnableVertexAttribArray(pos);
}

int main() {
    baseInit();
    auto window = createMainWindow(800, 600);
    glfwSetKeyCallback(window, key_callback);
    glfwSetDropCallback(window, drop_callback);
    // clang-format off
    //VertexBuffer first {
    //    0.5f, 0.5f, 0.0f,  // Верхний правый угол
    //    0.5f, -0.5f, 0.0f,  // Нижний правый угол
    //    -0.5f, 0.5f, 0.0f   // Верхний левый угол
    //};
    //VertexBuffer second {
    //    0.5f, -0.5f, 0.0f,  // Нижний правый угол
    //    -0.5f, -0.5f, 0.0f,  // Нижний левый угол
    //    -0.5f, 0.5f, 0.0f   // Верхний левый угол
    //};
    //IndexBuffer indices {  // Помните, что мы начинаем с 0!
    //    0, 1, 3,   // Первый треугольник
    //    1, 2, 3    // Второй треугольник
    //};
    
    VertexBuffer shape_and_color = {
        // Positions         // Colors
         0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f,  // Bottom Right
        -0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,  // Bottom Left
         0.0f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f   // Top 
    };
    // clang-format on

    ShaderProgram program;
    {
        ShaderType vertex(glsl::vertex, GL_VERTEX_SHADER);
        ShaderType fragm(glsl::fragm, GL_FRAGMENT_SHADER);
        /* ShaderType yelow(glsl::yelow, GL_FRAGMENT_SHADER);
         ShaderType uniform(glsl::uniform, GL_FRAGMENT_SHADER);*/

        program.attachShader(vertex);
        program.attachShader(fragm);
        program.compile();
    }
    VertexArrays<2> VAOs;

    // ================================
    // First Triangle setup
    // ===============================
    VAOs.on<0>([&shape_and_color] {
        // Копируем наш массив вершин в буфер для OpenGL
        shape_and_color.bind(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
        // Устанавливаем указатели на вершинные атрибуты
        setAttribute<GLfloat>(0, 3, GL_FALSE, 6, 0);
        setAttribute<GLfloat>(1, 3, GL_FALSE, 6, 3);
    });
    // ================================
    // Second Triangle setup
    // ===============================
    // VAOs.on<1>([&second] {
    //    // Копируем наш массив вершин в буфер для OpenGL
    //    second.bind(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
    //    // indices.bind(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);
    //    // Устанавливаем указатели на вершинные атрибуты
    //    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
    //    glEnableVertexAttribArray(0);
    //});

    while(!glfwWindowShouldClose(window)) {
        // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response
        // functions
        glfwPollEvents();
        // Render
        // Clear the colorbuffer
        glClearColor(0.2f, 0.3f, 0.3f, 0.4f);
        glClear(GL_COLOR_BUFFER_BIT);

        /*       GLfloat timeValue = glfwGetTime();
               GLfloat greenValue = (sin(timeValue) / 2) + 0.5;
               GLfloat rVal = (sin(timeValue * 3) / 2) + 0.5;
               GLfloat bVal = (sin(timeValue * 5) / 2) + 0.5;
               GLint vertexColorLocation = program.uniformLoaction("ourColor");*/

        // Now when we want to draw the triangle we first use the vertex and orange fragment shader from the first
        // program.
        program.run();
        // glUniform4f(vertexColorLocation, rVal, greenValue, bVal, 1.0f);
        // Draw the first triangle using the data from our first VAO
        VAOs.on<0>([] { glDrawArrays(GL_TRIANGLES, 0, 3); });

        // y_triangle.run();
        // VAOs.on<1>([] { glDrawArrays(GL_TRIANGLES, 0, 3); });

        /*glUseProgram(program1);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);*/

        // glBindVertexArray(0);
        glfwSwapBuffers(window);
    }
    glfwTerminate();
    return 0;
}
