#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;

out vec3 v_TexCoord;

uniform mat4 u_ViewProjection;

void main()
{
	gl_Position = u_ViewProjection * vec4(a_Position, 1);

	v_TexCoord = a_Position;
}

#type fragment
#version 330 core
out vec4 FragColor;

in vec3 v_TexCoord;

uniform samplerCube u_EnvTexture;
uniform float u_EnvMapRotation;

vec3 RotateVectorAboutY(float angle, vec3 vec)
{
    angle = radians(angle);
    mat3 rotationMatrix = mat3(vec3(cos(angle),0.0,sin(angle)),
                            vec3(0.0,1.0,0.0),
                            vec3(-sin(angle),0.0,cos(angle)));
    return rotationMatrix * vec;
}

void main()
{
	FragColor = texture(u_EnvTexture, RotateVectorAboutY(u_EnvMapRotation, v_TexCoord));
}