#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;

out VS_OUT {
    vec3 WorldPos;
    vec2 TexCoord;
    mat3 TBN;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform sampler2D displacementmap;
uniform float displacement_coefficient;

void main()
{
    // TODO HW8 - 0_displacement_normal | calculate displacement

    // read out displacement texture on corresponding point
    vec3 displacement = texture(displacementmap, aTexCoord).rgb;
    //vs_out.WorldPos = vs_out.WorldPos + displacement * displacement_coefficient;

    vs_out.TexCoord = aTexCoord;
	
	mat3 normalMatrix = transpose(inverse(mat3(model)));
	

    // TODO HW8 - 0_displacement_normal | calculate TBN
    vec3 N = normalize(normalMatrix * aNormal);
    vec3 T = normalize(normalMatrix * aTangent);
	vec3 B = cross(N, T);
	vs_out.TBN = mat3(T, B, N);

    vec4 worldPos = model * vec4((aPos + aNormal * displacement[0] * displacement_coefficient), 1.0);
    vs_out.WorldPos = worldPos.xyz / worldPos.w;

    gl_Position = projection * view * worldPos;
}
