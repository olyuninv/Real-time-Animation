#version 330 core
layout (location = 0) in vec3 position;

uniform vec3 objectColor;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 model;

out vec3 color;

void main()
{
    gl_Position = proj * view * model * vec4 (position, 1.0); //proj * view * model * 
	color = objectColor;
}