#version 460 core
out vec4 color;

uniform vec4 ourColor; // ћы устанавливаем значение этой переменной в коде OpenGL.

void main()
{
    color = ourColor;
}  
