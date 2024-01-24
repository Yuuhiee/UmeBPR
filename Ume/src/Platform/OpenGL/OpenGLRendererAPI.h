#pragma once

#include "Ume/Renderer/RendererAPI.h"

namespace Ume
{
	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		virtual void Init() override;
		virtual void Clear() override;
		virtual void SetClearColor(float r, float g, float b, float a) override;
		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0, bool depthTest = true) override;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
	};
}
