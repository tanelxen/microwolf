#shader vertex
#version 410 core

uniform mat4 MVP;
layout (location = 0) in vec4 position;
layout (location = 1) in vec2 in_texCoord;
layout (location = 2) in vec2 in_lmapCoord;
layout (location = 3) in vec3 normal;
layout (location = 4) in vec4 v_color;

out vec2 g_TexCoord;
out vec2 g_LmapCoord;

out vec4 g_vertColor;

void main()
{
    gl_Position = MVP * position;

    g_TexCoord = vec2(in_texCoord.x, in_texCoord.y);
    g_LmapCoord = vec2(in_lmapCoord.x, in_lmapCoord.y);
    g_vertColor = v_color;
}

#shader fragment
#version 410 core

in vec2 g_TexCoord;
in vec2 g_LmapCoord;
in vec4 g_vertColor;

//Texture samplers
uniform sampler2D s_bspTexture;
uniform sampler2D s_bspLightmap;

uniform float u_AlphaCutoff;
uniform bool u_hasLightmap;

//final color
out vec4 FragColor;

vec3 adjustExposure(vec3 color, float value) {
    return (1.0 + value) * color;
}

void main()
{
    vec4 o_texture  = texture(s_bspTexture, g_TexCoord);
    
    if (o_texture.a < u_AlphaCutoff) discard;
    
    vec4 light = g_vertColor;
    
    if (u_hasLightmap) {
        light = texture(s_bspLightmap, g_LmapCoord);
    }
    
    FragColor = o_texture * light;
    
    FragColor.rgb = adjustExposure(FragColor.rgb, 3);
}
