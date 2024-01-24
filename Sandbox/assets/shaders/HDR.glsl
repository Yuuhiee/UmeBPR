#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_Texcoord;

out vec2 v_TexCoord;

void main()
{
    v_TexCoord = a_Texcoord;
    gl_Position = vec4(a_Position.xy, 1.0, 1.0);
}

#type fragment
#version 330 core
out vec4 FragColor;

in vec2 v_TexCoord;

uniform sampler2D u_Texture;
uniform float u_Exposure;
uniform bool u_UseHDR;
uniform bool u_UseGamma;

const float pureWhite = 1.0;

vec3 GetHDR(vec3 color)
{
    color = color * u_Exposure;
    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
	float mappedLuminance = (luminance * (1.0 + luminance / (pureWhite * pureWhite))) / (1.0 + luminance);

	// Scale color by  of average luminances.
	return (mappedLuminance / luminance) * color;
}

vec3 GammaCorrect(vec3 color)
{
    return pow(color, vec3(1.0 / 2.2));
}

void main()
{
    vec3 color = texture(u_Texture, v_TexCoord).rgb;
    if (u_UseHDR)
        color = GetHDR(color);
    if (u_UseGamma)
        color = GammaCorrect(color);
    FragColor = vec4(color, 1.0);
}