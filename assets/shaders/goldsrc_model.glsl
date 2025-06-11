#shader vertex
#version 410 core
layout (location = 0) in vec4 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in uint boneIndex;

uniform mat4 uBoneTransforms[128];
uniform mat4 uModel;
uniform mat4 uMVP;

out vec2 uv;
out vec3 transformedNormal;
out vec4 transformedPosition;

void main()
{
    transformedPosition = uBoneTransforms[boneIndex] * position;
    transformedNormal = normalize(mat3(uModel) * mat3(uBoneTransforms[boneIndex]) * normal);
    gl_Position = uMVP * transformedPosition;
    uv = texCoord;
}

#shader fragment
#version 410 core
in vec2 uv;
in vec3 transformedNormal;
in vec4 transformedPosition;

//Texture samplers
uniform sampler2D s_texture;

uniform vec3 u_ambient;
uniform vec3 u_color;
uniform vec3 u_dir;

//final color
out vec4 FragColor;

vec3 adjustExposure(vec3 color, float value) {
    return (1.0 + value) * color;
}

vec4 fromLinear(vec4 linearRGB)
{
    bvec4 cutoff = lessThan(linearRGB, vec4(0.0031308));
    vec4 higher = vec4(1.055)*pow(linearRGB, vec4(1.0/2.4)) - vec4(0.055);
    vec4 lower = linearRGB * vec4(12.92);

    return mix(higher, lower, cutoff);
}

void main()
{
    float nDotL = max(dot(u_dir, transformedNormal), 0);
    
    vec3 light = u_ambient + u_color * nDotL;
    light = clamp(light, 0.0, 1.0);

    FragColor = texture(s_texture, uv) * vec4(light, 1.0);
    
//    FragColor = fromLinear(FragColor);
}
