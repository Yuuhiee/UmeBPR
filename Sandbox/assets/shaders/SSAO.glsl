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

uniform sampler2D u_PositionBuffer;
uniform sampler2D u_NormalBuffer;
uniform sampler2D u_NoiseTexture;
uniform mat4 u_Projection;

uniform int u_Width;
uniform int u_Height;

uniform vec3 u_Samples[64];

const int SAMPLES = 64;
float radius = 10;

void main()
{
    vec3 fragPos = texture(u_PositionBuffer, v_TexCoord).xyz;
    vec3 normal = texture(u_NormalBuffer, v_TexCoord).xyz;
    
    vec3 noise = texture(u_NoiseTexture, v_TexCoord * vec2(u_Width, u_Height) / 4.0).xyz;
    vec3 tangent = normalize(noise - normal * dot(noise, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for (int i = 0; i < SAMPLES; ++i)
    {
        vec3 sample = TBN * u_Samples[i];
        sample = fragPos + sample * radius;
        
        vec4 offset = vec4(sample, 1.0);
        offset = u_Projection * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // get sample depth
        float sampleDepth = -texture(u_PositionBuffer, offset.xy).w; // Get depth value of kernel sample
        
        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth ));
        occlusion += (sampleDepth >= sample.z ? 1.0 : 0.0) * rangeCheck;           
        // occlusion += sampleDepth <= sample.z ? 1.0 : 0.0;           
    }
    occlusion = 1.0 - (occlusion / float(SAMPLES));
    // occlusion = (occlusion / float(SAMPLES));

    FragColor = vec4(vec3(occlusion), 1.0);
    // FragColor = vec4(mix(vec3(occlusion), fragPos, 0.99), 1.0);
}