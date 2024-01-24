#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec3 v_Position;

void main()
{
    vec4 position = u_View * u_Model * vec4(a_Position, 1.0);
	v_Position = vec3(position);
    gl_Position = u_Projection * position;
}

#type fragment
#version 330 core

layout(location = 0) out vec4 FragColor;

in vec3 v_Position;

const float near = 0.1;
const float far = 10000.0;

float LinearDepth(float z)
{
    z = z * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
    FragColor.xyz = v_Position;
    FragColor.w = LinearDepth(gl_FragCoord.z);
}
