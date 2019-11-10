#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <ostream>
#include <iostream>

GLFWwindow* createMainWindow(int width, int height, const char* title = nullptr) {
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
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    // Когда пользователь нажимает ESC, мы устанавливаем свойство WindowShouldClose в true, 
    // и приложение после этого закроется
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

int main() {
    auto window = createMainWindow(800, 600);
    glfwSetKeyCallback(window, key_callback);
    //int i = 0;

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        //....
        
        glClearColor(0.2f, 0.3f, 0.3f, 0.4f);
        glClear(GL_COLOR_BUFFER_BIT);
        //i++;

        glfwSwapBuffers(window);
    }
    glfwTerminate();
    return 0;
}
