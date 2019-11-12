#version 460 core

layout (location = 0) in vec3 aPos; // Устанавливаем позиция переменной с координатами в 0
//layout (location = 1) in vec3 color;    // А позицию переменной с цветом в 1

out vec3 ourColor; // Передаем цвет во фрагментный шейдер
uniform float offset_x;
uniform float offset_y;
uniform vec3 upd;
void main()
{
    //gl_Position = vec4(aPos.x + offset_x, aPos.y+offset_y, aPos.z, 1.0);
    gl_Position = vec4(aPos.x + upd.x, aPos.y+upd.y, aPos.z, 1.0);
    ourColor = vec3(upd.z, 0.0, 0.1); // Устанавливаем значение цвета, полученное от вершинных данных
}   
