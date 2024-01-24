#pragma once

#include <Ume.h>

#include "glm/glm.hpp"

using namespace Ume;

class Sandbox3D : public Layer
{
public:
	Sandbox3D();
	~Sandbox3D() = default;

	virtual void OnUpdate(Timestep ts) override;
	virtual void OnEvent(Event& e) override;
	virtual void OnImGuiRender() override;
private:
	PerspectiveCameraController m_PerspectiveCameraController;
	Ref<Shader> m_TextureShader;
	Ref<VertexArray> m_CubeVA;
	Ref<Texture2D> m_DefaultTexture;
	Ref<Texture2D> m_LogoTexture;
	glm::vec3 m_CubeColorTint = { 1.0f, 1.0f, 1.0f };
};

class PBRLayer : public Layer
{
public:
	PBRLayer();
	~PBRLayer() = default;

	virtual void OnUpdate(Timestep ts) override;
	virtual void OnEvent(Event& e) override;
	virtual void OnImGuiRender() override;
private:
	Ref<Mesh> m_Mesh;
	Ref<Shader> m_Shader;
	PerspectiveCameraController m_CameraController;
	PerspectiveCameraSample m_CameraSample;
	PerspectiveCamera m_Camera;

	Ref<Texture2D> m_CheckerboardTex;
	Ref<Texture2D> m_AlbedoTex;
	Ref<Texture2D> m_NormalTex;
	Ref<Texture2D> m_MetalnessTex;
	Ref<Texture2D> m_RoughnessTex;
	Ref<Texture2D> m_BRDFLUT;
	Ref<Texture2D> m_EmuLUT;
	Ref<Texture2D> m_EavgLUT;
	Ref<TextureCube> m_EnvRadiance;
	Ref<TextureCube> m_EnvIrradiance;
private:
	Ref<VertexArray> m_CubeVA;
	Ref<Texture2D> m_DefaultTexture;
	Ref<Shader> m_TextureShader;
	Ref<Shader> m_GShader;
	Ref<Shader> m_SSAOShader;
	Ref<Texture2D> m_SSAONoise;

	struct RenderPass
	{
		std::string Name;
		Framebuffer* FrameBuffer;
	};
	std::vector<RenderPass> m_RenderPasses;
	int m_FinalPass;
	inline RenderPass& GetFinalPass() { return m_RenderPasses[m_FinalPass]; }
private:
	struct Light
	{
		glm::vec3 Position;
		glm::vec3 Color;
		float Intensity;
	};
	struct DirectionalLight : public Light
	{
		glm::vec3 Direction;
	};
	DirectionalLight m_Light;
	float m_Fresnel = 0.04f;
	bool m_UseAlbedoTex = false;
	bool m_UseNormalTex = false;
	bool m_UseMetalnessTex = false;
	bool m_UseRoughnessTex = false;
	bool m_ReceiveEnv = true;
	bool m_AlbedoSRGB = true;
	float m_NormalStrengh = 1.0f;
	float m_Metalness = 1.0f;
	float m_Roughness = 1.0f;
	float m_RadiancePrefilter = 0.2f;
	float m_EnvMapRotation = 0.0f;
	glm::vec3 m_Albedo = { 0.972f, 0.96f, 0.915f };
	glm::vec3 m_AlbedoTint = glm::vec3(1.0f);
};