#type vertex
#version 430 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Binormal;
layout(location = 4) in vec2 a_Texcoord;

out VertexOuput
{
	vec2 Texcoord;
    vec3 TanPosition;
    vec3 TanLightDir;
    vec3 TanViewDir;
    vec3 WorldViewDir;
    mat3 NormalMatrix;
} vs_Out;

uniform mat4 u_Model;
uniform mat4 u_ViewProjection;
uniform mat4 u_NormalMatrix;
uniform vec3 u_LightDirection;
uniform vec3 u_CameraPosition;

void main()
{
	vec4 position = u_Model * vec4(a_Position, 1.0);

    vec3 T = normalize(a_Tangent);
    vec3 B = normalize(a_Binormal);
    vec3 N = normalize(a_Normal);
    mat3 TBN = transpose(mat3(u_NormalMatrix) * mat3(T, B, N));

	vs_Out.Texcoord = a_Texcoord;
    vs_Out.TanPosition = TBN * position.xyz;
    vs_Out.TanLightDir = TBN * u_LightDirection;
    vs_Out.TanViewDir = TBN * (u_CameraPosition - position.xyz);
    vs_Out.WorldViewDir = u_CameraPosition - position.xyz;
    vs_Out.NormalMatrix = mat3(u_NormalMatrix) * mat3(T, B, N);

	gl_Position = u_ViewProjection * position;
}

#type fragment
#version 430 core

in VertexOuput
{
	vec2 Texcoord;
    vec3 TanPosition;
    vec3 TanLightDir;
    vec3 TanViewDir;
    vec3 WorldViewDir;
    mat3 NormalMatrix;
} vs_In;

layout(location = 0) out vec4 FragColor;

uniform sampler2D u_AlbedoTexture;
uniform sampler2D u_NormalTexture;
uniform sampler2D u_MetalnessTexture;
uniform sampler2D u_RoughnessTexture;
uniform sampler2D u_BRDFLUTTexture;
uniform samplerCube u_EnvRadianceTex;
uniform samplerCube u_EnvIrradianceTex;
uniform sampler2D u_EmuLUT;
uniform sampler2D u_EavgLUT;

uniform vec3 u_LightColor;
uniform float u_LightIntensity;

uniform bool u_UseAlbedoTex;
uniform bool u_UseNormalTex;
uniform bool u_UseMetalnessTex;
uniform bool u_UseRoughnessTex;
uniform bool u_ReceiveEnvLight;

uniform vec3 u_Albedo;
uniform vec3 u_AlbedoTint;
uniform float u_NormalStrengh;
uniform float u_Roughness;
uniform float u_Metalness;
uniform float u_Fresnel;
uniform float u_RadiancePrefilter;
uniform float u_EnvMapRotation;

#define PI 3.141592653

struct Params
{
    vec3 Albedo;
    vec3 Normal;
    vec3 WorldNormal;
    float Metalness;
    float Roughness;
    float NoH;
    float NoL;
    float VoH;
    float NoV;
};

Params m_Params;

float D_GGX(float NoH, float roughness)
{
    float a2 = pow(roughness, 4.0);
    float NoH2 = NoH * NoH;
    return pow(roughness, 4.0) / (PI * pow(NoH2 * (a2 - 1.0) + 1.0, 2.0));
}

vec3 F_Schlick(float VoH, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - VoH, 0.0, 1.0), 5.0);
}

vec3 F_SchlickRoughness(float VoH, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - VoH, 5.0);
}

float GeometrySchlickGGX(float NdotV, float k)
{
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return nom / denom;
}

float G_Smith(float NoV, float NoL, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return GeometrySchlickGGX(NoV, k) * GeometrySchlickGGX(NoL, k);
}

vec3 AverageFresnel(vec3 r, vec3 g)
{
    return vec3(0.087237) + 0.0230685*g - 0.0864902*g*g + 0.0774594*g*g*g
           + 0.782654*r - 0.136432*r*r + 0.278708*r*r*r
           + 0.19744*g*r + 0.0360605*g*g*r - 0.2586*g*r*r;
}

vec3 MultiScatterBRDF()
{
    vec3 E_o = texture2D(u_EmuLUT, vec2(m_Params.NoL, m_Params.Roughness)).xyz;
    vec3 E_i = texture2D(u_EmuLUT, vec2(m_Params.NoV, m_Params.Roughness)).xyz;

    vec3 E_avg = texture2D(u_EavgLUT, vec2(0, m_Params.Roughness)).xyz;
    // copper
    vec3 edgetint = vec3(0.827, 0.792, 0.678);
    vec3 F_avg = AverageFresnel(m_Params.Albedo, edgetint);
    
    vec3 F_ms = (1.0 - E_o) * (1.0 - E_i) / (PI * (1.0 - E_avg));
    vec3 F_add = F_avg * E_avg / (1.0 - F_avg * (1.0 - E_avg));

    return F_ms * F_add;
}

vec3 Lighting(vec3 F0)
{
    float D = D_GGX(m_Params.NoH, m_Params.Roughness);
    float G = G_Smith(m_Params.NoV, m_Params.NoL, m_Params.Roughness);
    vec3 F = F_Schlick(m_Params.VoH, F0);

    vec3 kd = (1.0 - F) * (1.0 - m_Params.Metalness);

    vec3 lightInradiance = u_LightColor * u_LightIntensity;
    vec3 Fmicro = (D * F * G) / (4.0 * m_Params.NoL * m_Params.NoV + 0.0001);
    vec3 brdf = Fmicro;
    brdf += kd * m_Params.Albedo / PI;
    // brdf += MultiScatterBRDF();
    vec3 Lo = brdf * (lightInradiance * m_Params.NoL);
    return Lo;
}

float RadicalInverse_VdC(uint bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}
vec3 ImportanceSampleGGX(vec2 Xi, float Roughness, vec3 N)
{
	float a = Roughness * Roughness;
	float Phi = 2 * PI * Xi.x;
	float CosTheta = sqrt( (1 - Xi.y) / ( 1 + (a*a - 1) * Xi.y ) );
	float SinTheta = sqrt( 1 - CosTheta * CosTheta );
	vec3 H;
	H.x = SinTheta * cos( Phi );
	H.y = SinTheta * sin( Phi );
	H.z = CosTheta;
	vec3 UpVector = abs(N.z) < 0.999 ? vec3(0,0,1) : vec3(1,0,0);
	vec3 TangentX = normalize( cross( UpVector, N ) );
	vec3 TangentY = cross( N, TangentX );
	// Tangent to world space
	return TangentX * H.x + TangentY * H.y + N * H.z;
}
vec3 PrefilterEnvMap(float Roughness, vec3 R)
{
	vec3 N = R;
	vec3 V = R;
	vec3 PrefilteredColor = vec3(0.0);
	const int NumSamples = 1024;
	// const int NumSamples = 128;
    float TotalWeight = 0.0;
    int maxMipLevel = textureQueryLevels(u_EnvRadianceTex) - 1;
    int i = 0;
	for(; i < NumSamples; i++)
	{
		vec2 Xi = Hammersley(i, NumSamples);
		vec3 H = ImportanceSampleGGX(Xi, Roughness, N);
		// vec3 L = vec3(2) * dot(V, H) * H - V;
		vec3 L = normalize(reflect(-V, H));
		float NoL = clamp(dot(N, L), 0.0, 1.0);
		if (NoL > 0)
		{
			PrefilteredColor += texture(u_EnvRadianceTex, L).rgb * NoL;
			//PrefilteredColor += textureLod(u_EnvRadianceTex, L, 0).rgb * NoL;
			TotalWeight += NoL;
		}
	}
	return PrefilteredColor / TotalWeight;
}
vec3 RotateVectorAboutY(float angle, vec3 vec)
{
    angle = radians(angle);
    mat3 rotationMatrix = mat3(vec3(cos(angle),0.0,sin(angle)),
                            vec3(0.0,1.0,0.0),
                            vec3(-sin(angle),0.0,cos(angle)));
    return rotationMatrix * vec;
}
vec3 IBL(vec3 F0)
{
    // vec3 R = reflect(-vs_In.WorldViewDir, vs_In.WorldNormal);
    vec3 irradiance = texture(u_EnvIrradianceTex, m_Params.WorldNormal).rgb;
    vec3 F = F_SchlickRoughness(m_Params.VoH, F0, m_Params.Roughness);
    vec3 kd = (1.0 - F) * (1.0 - m_Params.Metalness);
    vec3 diffuse = irradiance * m_Params.Albedo * kd;

    int u_EnvRadianceTexLevels = textureQueryLevels(u_EnvRadianceTex);

    vec3 R = reflect(-vs_In.WorldViewDir, m_Params.WorldNormal);
    R = RotateVectorAboutY(u_EnvMapRotation, R);
    vec2 ks = texture(u_BRDFLUTTexture, vec2(m_Params.NoV, 1.0 - m_Params.Roughness)).rg;

	vec3 specularIrradiance = vec3(0.0);
    // specularIrradiance = PrefilterEnvMap(m_Params.Roughness * m_Params.Roughness, R) * 1.0;
    if (u_RadiancePrefilter > 0.5)
		specularIrradiance = PrefilterEnvMap(m_Params.Roughness, R) * u_RadiancePrefilter;
	else
		specularIrradiance = textureLod(u_EnvRadianceTex, R, sqrt(m_Params.Roughness) * u_EnvRadianceTexLevels).rgb * (1.0 - u_RadiancePrefilter);

    vec3 specular = specularIrradiance * (F * ks.x + ks.y);

    return diffuse + specular;
}
void main()
{
    m_Params.Albedo = u_UseAlbedoTex ? texture(u_AlbedoTexture, vs_In.Texcoord).rgb * u_AlbedoTint : u_Albedo;
    m_Params.Metalness = u_UseMetalnessTex ? texture(u_MetalnessTexture, vs_In.Texcoord).r : 1.0;
    m_Params.Metalness *= u_Metalness;
    m_Params.Roughness = u_UseRoughnessTex ? max(texture(u_RoughnessTexture, vs_In.Texcoord).r, 0.05) : 1.0;
    m_Params.Roughness *= u_Roughness;
    m_Params.Normal = u_UseNormalTex ? texture(u_NormalTexture, vs_In.Texcoord).rgb * 2.0 - 1.0 : vec3(0.0, 0.0, 1.0);
    m_Params.Normal = normalize(m_Params.Normal * vec3(1.0, 1.0, u_NormalStrengh));
    m_Params.WorldNormal = normalize(vs_In.NormalMatrix * m_Params.Normal);

    vec3 N = m_Params.Normal;
    vec3 L = normalize(vs_In.TanLightDir);
    vec3 V = normalize(vs_In.TanViewDir);
    vec3 H = normalize(L + V);

    vec3 F0 = vec3(u_Fresnel);
    F0 = mix(F0, m_Params.Albedo, m_Params.Metalness);

    m_Params.NoH = max(dot(N, H), 0.0);
    m_Params.NoL = max(dot(N, L), 0.0);
    m_Params.VoH = dot(V, H);
    m_Params.NoV = max(dot(N, V), 0.0);

    vec3 lightContribution = vec3(0.0);
    lightContribution =Lighting(F0);

    vec3 iblContribution = vec3(0.0);
    if (u_ReceiveEnvLight)
        iblContribution = IBL(F0);

    vec3 color;
    color = lightContribution + iblContribution;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));  
    FragColor = vec4(color, 1.0);
}
