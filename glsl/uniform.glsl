#version 460 core
out vec4 color;

uniform vec4 ourColor; // �� ������������� �������� ���� ���������� � ���� OpenGL.

void main()
{
    color = ourColor;
}  
