#shader vertex
#version 410 core

layout (location = 0) in vec4 position;
layout (location = 1) in vec3 normal;

uniform mat4 MVP;
uniform vec3 u_AmbientCube[6];

out vec3 ambient;

vec3 AmbientLight(in vec3 t_NormalWorld);

void main()
{
    gl_Position = MVP * position;
    ambient = AmbientLight(normal);
}

vec3 AmbientLight(in vec3 t_NormalWorld)
{
    vec3 t_Weight = t_NormalWorld * t_NormalWorld;
    bvec3 t_Negative = lessThan(t_NormalWorld, vec3(0.0));
    return (
            t_Weight.x * u_AmbientCube[t_Negative.x ? 0 : 1].rgb +
            t_Weight.y * u_AmbientCube[t_Negative.y ? 2 : 3].rgb +
            t_Weight.z * u_AmbientCube[t_Negative.z ? 5 : 4].rgb
            );
}

#shader fragment
#version 410 core

in vec3 ambient;

//final color
out vec4 FragColor;

void main()
{
    FragColor = vec4(ambient, 1.0);
}
