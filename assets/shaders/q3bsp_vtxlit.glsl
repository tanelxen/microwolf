#shader vertex
#version 410 core

uniform mat4 MVP;
layout (location = 0) in vec4 position;
layout (location = 1) in vec2 in_texCoord;
layout (location = 2) in vec2 in_lmapCoord;
layout (location = 3) in vec4 v_color;

out vec2 g_TexCoord;
out vec4 g_vertColor;

void main()
{
    gl_Position = MVP * position;

    g_TexCoord = vec2(in_texCoord.x, in_texCoord.y);
    g_vertColor = v_color;
}

#shader fragment
#version 410 core

in vec2 g_TexCoord;
in vec4 g_vertColor;

//Texture samplers
uniform sampler2D s_bspTexture;

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
    
    FragColor = o_texture * g_vertColor;
    
    FragColor.rgb = adjustExposure(FragColor.rgb, 3);
    
//    float gamma = 2.0;
//    FragColor.rgb = pow(FragColor.rgb, vec3(1.0/gamma));
//    FragColor = fromLinear(FragColor);
}
