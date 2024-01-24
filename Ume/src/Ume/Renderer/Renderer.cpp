#include "umepch.h"
#include "Renderer.h"

#include "RenderCommand.h"
#include "Renderer2D.h"

namespace Ume
{
	Scope<Renderer::SceneData> Renderer::s_SceneData = nullptr;
	Ref<ShaderLibrary> Renderer::s_ShaderLibrary = nullptr;
	RenderCommandQueue Renderer::s_CommandQueue;
	Scope<RendererAPI> Renderer::s_RendererAPI = RendererAPI::Create();

	void Renderer::Init()
	{
		s_SceneData = CreateScope<SceneData>();
		s_ShaderLibrary = CreateRef<ShaderLibrary>();

		RenderCommand::Init();
		Renderer2D::Init();
	}

	void Renderer::StartScene(const Camera& camera)
	{
		s_SceneData->ViewProjection = camera.GetViewProjectionMatrix();
	}

	void Renderer::EndScene()
	{
	}

	void Renderer::Submit(const Ref<VertexArray>& vertexArray, bool depthTest)
	{
		/*const uint32_t size = sizeof(glm::mat4) * 2;
		UniformBufferDeclaration<size, 2> uniformBuffer;
		uniformBuffer.Push("u_Model", modelMatrix);
		uniformBuffer.Push("u_ViewProjection", s_SceneData->ViewProjection);
		shader->Bind();
		shader->UploadUniformBuffer(uniformBuffer);*/

		UME_RENDER_2(vertexArray, depthTest, {
			vertexArray->Bind();
			vertexArray->GetIndexBuffer()->Bind();
			RenderCommand::DrawIndexed(vertexArray, 0, depthTest);
		});
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		s_RendererAPI->SetViewport(0, 0, width, height);
	}

	void Renderer::Clear()
	{
		UME_RENDER(s_RendererAPI->Clear(););
	}

	void Renderer::SetClearColor(const glm::vec4& color)
	{
		UME_RENDER_1(color, s_RendererAPI->SetClearColor(color.r, color.g, color.b, color.a););
	}
}