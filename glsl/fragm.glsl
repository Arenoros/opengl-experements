#version 460 core

in vec3 ourColor; // Входная переменная из вершинного шейдера (то же название и тот же тип)

out vec4 color;

void main()
{
    color = vec4(ourColor, 1.0f);
} 
