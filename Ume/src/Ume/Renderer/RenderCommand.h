#pragma once

#include "VertexArray.h"
#include "RendererAPI.h"
namespace Ume
{
	class RenderCommand
	{
	public:
		inline static void Init()
		{
			s_RendererAPI->Init();
		}

		inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			s_RendererAPI->SetViewport(x, y, width, height);
		}

		inline static void SetClearColor(float r, float g, float b, float a)
		{
			s_RendererAPI->SetClearColor(r, g, b, a);
		}

		inline static void Clear()
		{
			s_RendererAPI->Clear();
		}

		inline static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0, bool depthTest = true)
		{
			s_RendererAPI->DrawIndexed(vertexArray, indexCount, depthTest);
		}
	private:
		static Scope<RendererAPI> s_RendererAPI;
	};
}

