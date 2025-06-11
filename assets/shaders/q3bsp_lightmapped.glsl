#shader vertex
#version 410 core

uniform mat4 MVP;
layout (location = 0) in vec4 position;
layout (location = 1) in vec2 in_texCoord;
layout (location = 2) in vec2 in_lmapCoord;

out vec2 g_TexCoord;
out vec2 g_LmapCoord;

void main()
{
    gl_Position = MVP * position;

    g_TexCoord = vec2(in_texCoord.x, in_texCoord.y);
    g_LmapCoord = vec2(in_lmapCoord.x, in_lmapCoord.y);
}

#shader fragment
#version 410 core

in vec2 g_TexCoord;
in vec2 g_LmapCoord;

//Texture samplers
uniform sampler2D s_bspTexture;
uniform sampler2D s_bspLightmap;

//final color
out vec4 FragColor;

vec3 adjustExposure(vec3 color, float value) {
    return (1.0 + value) * color;
}

//vec4 gammaCorrect(vec4 color) {
//    return pow(color, vec4(1.0 / 2.0));
//}

vec4 fromLinear(vec4 linearRGB)
{
    bvec4 cutoff = lessThan(linearRGB, vec4(0.0031308));
    vec4 higher = vec4(1.055)*pow(linearRGB, vec4(1.0/2.4)) - vec4(0.055);
    vec4 lower = linearRGB * vec4(12.92);

    return mix(higher, lower, cutoff);
}

//vec4 toLinear(vec4 sRGB)
//{
//    bvec4 cutoff = lessThan(sRGB, vec4(0.04045));
//    vec4 higher = pow((sRGB + vec4(0.055))/vec4(1.055), vec4(2.4));
//    vec4 lower = sRGB/vec4(12.92);
//
//    return mix(higher, lower, cutoff);
//}

void main()
{
    vec4 o_texture  = texture(s_bspTexture,  g_TexCoord);
    vec4 o_lightmap = texture(s_bspLightmap, g_LmapCoord);
    
    FragColor = o_texture * o_lightmap;
    
    FragColor.rgb = adjustExposure(FragColor.rgb, 3);
    
//    float gamma = 2.0;
//    FragColor.rgb = pow(FragColor.rgb, vec3(1.0/gamma));
//    FragColor = fromLinear(FragColor);
}
