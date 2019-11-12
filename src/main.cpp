#include "stdafx.h"
#include "api.h"
#include <thread>
#include <mutex>

struct AnalizeFile {
    double colors[64];
    GLfloat buf_colors[64];
    std::string path;
    std::thread* th;
    std::mutex mtx;
    bool stop = false;
    void set_path(std::string _path) { path = std::move(_path); }
    void thread_fun() {
        File file(path, "rb");
        memset(colors, 0, sizeof(colors));
        GLfloat step = 0.000001f;
        uint64_t buf[1024];
        GLfloat normalize[64];
        while(file.read(buf, sizeof(buf)) && !stop) {
            for(int i = 0; i < 1024; i++) {
                uint64_t v = 1;
                for(int j = 0; j < 64; ++j) {
                    if(v & buf[i])
                        colors[j] += 0.01;
                    v <<= 1;
                }
            }
            auto [min, it] = std::minmax_element(colors, colors+ 64);
            double dst = *it - *min;
            double max = dst != 0.f? 1.f / dst : dst;
            for(int j = 0; j < 64; ++j) { normalize[j] = (colors[j]-*min) * max; }

            std::lock_guard<std::mutex> lock(mtx);
            std::memcpy(buf_colors, normalize, sizeof(normalize));
        }
    }
    AnalizeFile(): colors{0}, buf_colors{0}, th(nullptr) {}
    ~AnalizeFile() {
        if(th) th->join();
        delete th;
    }
    void Stop() {
        stop = true;
        if(th) th->join();
        delete th;
        th = nullptr;
    }
    void Colors(GLfloat* dst) {
        std::lock_guard<std::mutex> lock(mtx);
        std::memcpy(dst, buf_colors, sizeof(buf_colors));
    }
    void Run() {
        if(th) return;
        stop = false;
        th = new std::thread(&AnalizeFile::thread_fun, this);
    }
};
AnalizeFile analize;

void drop_callback(GLFWwindow* window, int count, const char** paths) {
    for(int i = 0; i < count; i++) std::cout << paths[i];
    if(count > 0) {
        analize.set_path(paths[0]);
        analize.Stop();
        analize.Run();
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    // Когда пользователь нажимает ESC, мы устанавливаем свойство WindowShouldClose в true,
    // и приложение после этого закроется
    if(action != GLFW_PRESS) return;
    switch(key) {
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GL_TRUE);
        break;
    case GLFW_KEY_S:
        analize.Stop();
        break;
    case GLFW_KEY_F:
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // не работает
        break;
    case GLFW_KEY_L:
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  // не работает
        break;
    }
}

int main() {
    baseInit();
    auto window = createMainWindow(800, 600);
    glfwSetKeyCallback(window, key_callback);
    glfwSetDropCallback(window, drop_callback);

    // clang-format off
    VertexBuffer shape_and_color = {
        // Positions         // Colors
        -0.6f,  -0.8f, 0.0f,   1.0f, 0.0f, 0.0f,  // Bottom Right
        -0.8f,  -0.8f, 0.0f,   0.0f, 1.0f, 0.0f,  // Bottom Left
        -0.8f,  -0.6f, 0.0f,   0.0f, 0.0f, 1.0f,  // Top Left
        -0.6f,  -0.6f, 0.0f,   1.0f, 0.0f, 1.0f   // Top Right
    };
    VertexBuffer square {
        // Positions         
        -0.6f, -0.8f, 0.0f, // Bottom Right
        -0.8f, -0.8f, 0.0f, // Bottom Left
        -0.8f, -0.6f, 0.0f, // Top Left
        -0.6f, -0.6f, 0.0f  // Top Right
    };
    IndexBuffer points {
        0,1,2,
        2,3,0
    };
    // clang-format on
    glsl::Program<glsl::shader> program;

    // Set up vertex data (and buffer(s)) and attribute pointers
    VertexArrays<64> VAOs;
    for(int i = 0; i < 64; ++i) {
        VAOs.on(i, [&square, &points] {
            square.bind(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
            points.bind(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);
            setVertexAttribute<GLfloat>(0, 3, GL_FALSE, 3, 0);
            // setVertexAttribute<GLfloat>(1, 3, GL_FALSE, 6, 3);
        });
    }
    /*VAOs.on<0>([&shape_and_color, &points] {
        shape_and_color.bind(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
        points.bind(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);
        setVertexAttribute<GLfloat>(0, 3, GL_FALSE, 6, 0);
        setVertexAttribute<GLfloat>(1, 3, GL_FALSE, 6, 3);
    });*/

    // Game loop
    GLfloat colors[64];
    GLfloat upd[] = {0, 0, 0};
    while(!glfwWindowShouldClose(window)) {
        // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response
        glfwPollEvents();

        analize.Colors(colors);
        glClearColor(0.2f, 0.3f, 0.3f, 0.4f);
        glClear(GL_COLOR_BUFFER_BIT);
        // auto offset = [](int i) -> GLfloat { return i*0.2f; };
        for(int i = 0; i < 8; ++i) {
            for(int j = 0; j < 8; ++j) {
                VAOs.bind(i * 8 + j);
                upd[0] = i * 0.2f;
                upd[1] = j * 0.2f;
                upd[2] = colors[i * 8 + j];
                program.use();
                program.set("upd", upd);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }
        // VAOs.on<0>([&] {
        //    GLfloat offset = sin(glfwGetTime()) * 0.5f;
        //    program.use();
        //    program.set("offset", offset);
        //    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        //    //glDrawArrays(GL_LINES, 0, 3);
        //});
        VAOs.unbind();
        // Swap the screen buffers
        glfwSwapBuffers(window);
    }
    glfwTerminate();
    return 0;
}
