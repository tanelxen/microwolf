#shader vertex
#version 410 core

layout (location = 0) in vec2 position;

uniform mat4 uProjMatrix;
uniform mat4 uModelMatrix;

void main()
{
    gl_Position = uProjMatrix * uModelMatrix * vec4(position, 0.0, 1.0);
}

#shader fragment
#version 410 core

//final color
out vec4 FragColor;

void main()
{
    FragColor = vec4(1, 0, 1, 1);
}
