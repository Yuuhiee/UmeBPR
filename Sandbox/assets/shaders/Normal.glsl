#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform mat4 u_NormalMatrix;

out vec3 v_Normal;

void main()
{
    gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0);
    v_Normal = normalize(mat3(u_NormalMatrix) * a_Normal);
}

#type fragment
#version 330 core

layout(location = 0) out vec4 FragColor;

in vec3 v_Normal;

void main()
{
    vec3 normal = normalize(v_Normal);
    FragColor = vec4(normal, 1.0);
}
