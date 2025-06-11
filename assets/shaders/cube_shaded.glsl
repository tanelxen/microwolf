#shader vertex
#version 410 core

uniform mat4 MVP;
layout (location = 0) in vec4 position;
layout (location = 1) in vec4 normal;

out float shade;

float shadeForNormal(vec4 normal);

void main()
{
    gl_Position = MVP * position;
    shade = shadeForNormal(normal);
}

const float lightaxis[3] = float[](0.6f, 0.8f, 1.0f);

float shadeForNormal(vec4 normal)
{
    int i;
    float f;

    // axial plane
    for ( i = 0; i < 3; i++ ) {
        if (abs(normal[i]) > 0.9) {
            f = lightaxis[i];
            return f;
        }
    }
    
    // between two axial planes
    for (i = 0; i < 3; i++) {
        if (abs(normal[i]) < 0.1) {
            f = ( lightaxis[( i + 1 ) % 3] + lightaxis[( i + 2 ) % 3] ) / 2;
            return f;
        }
    }
    
    // other
    f = ( lightaxis[0] + lightaxis[1] + lightaxis[2] ) / 3;
    return f;
}

#shader fragment
#version 410 core

uniform vec4 color;
in float shade;

//final color
out vec4 FragColor;

void main()
{
    FragColor = color * vec4(shade, shade, shade, 1.0);
}
