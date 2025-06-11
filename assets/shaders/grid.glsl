#shader vertex
#version 410 core

uniform mat4 MVP;
layout (location = 0) in vec4 position;
layout (location = 1) in vec3 in_color;

out vec2 g_uv;

void main()
{
    gl_Position = MVP * position;

    g_uv = position.xz;
}

#shader fragment
#version 410 core

in vec2 g_uv;

out vec4 FragColor;

vec4 grid(vec2 uv, float scale, float intensity, bool axis)
{
    vec2 coord = uv / scale;
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / (derivative + 0.0125);
    
    float line = min(grid.x, grid.y);
    float minimumz = min(derivative.y, 1) * scale;
    float minimumx = min(derivative.x, 1) * scale;
    
    vec4 color = vec4(intensity, intensity, intensity, 1.0 - min(line, 1.0));
    
    if (axis)
    {
        // z axis
        if(uv.x > -2 * minimumx && uv.x < 2 * minimumx) {
            color = vec4(0.0, 0.0, 1.0, 0.4);
        }
        
        // x axis
        if(uv.y > -2 * minimumz && uv.y < 2 * minimumz) {
            color = vec4(1.0, 0.0, 0.0, 0.4);
        }
    }
    
    return color;
}

void main()
{
    vec4 main = grid(g_uv, 8, 0.01, false);
    vec4 second = grid(g_uv, 32, 0.4, true);
    
    FragColor = main + second;
}
