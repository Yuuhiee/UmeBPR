#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

out vec2 v_TexCoord;

uniform mat4 u_Model;
uniform mat4 u_ViewProjection;

void main()
{
	gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1);
	//gl_Position = vec4(a_Position, 1);

	v_TexCoord = a_TexCoord;
}

#type fragment
#version 330 core

in vec2 v_TexCoord;

out vec4 FragColor;

uniform vec3 u_ColorTint;
uniform sampler2D u_Texture;

void main()
{
	vec4 albedo = texture(u_Texture, v_TexCoord);
	FragColor = albedo * vec4(u_ColorTint, 0.4);
}