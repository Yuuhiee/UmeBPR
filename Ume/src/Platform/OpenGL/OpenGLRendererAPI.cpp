#include "umepch.h"
#include "OpenGLRendererAPI.h"

#include "glad/glad.h"

namespace Ume
{
	void OpenGLRendererAPI::Init()
	{
		UME_PROFILE_FUNCTION();

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		auto& caps = RendererAPI::GetCapabilities();

		caps.Vendor = (const char*)glGetString(GL_VENDOR);
		caps.Renderer = (const char*)glGetString(GL_RENDERER);
		caps.Version = (const char*)glGetString(GL_VERSION);

		glGetIntegerv(GL_MAX_SAMPLES, &caps.MaxSamples);
		//glGetFloatv(GL_MAX_TEXTUEW_MAX, &caps.MaxAnisotropy);
	}

	void OpenGLRendererAPI::Clear()
	{
		UME_PROFILE_FUNCTION();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::SetClearColor(float r, float g, float b, float a)
	{
		UME_PROFILE_FUNCTION();

		glClearColor(r, g, b, a);
	}

	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount, bool depthTest)
	{
		UME_PROFILE_FUNCTION();

		if (depthTest)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);

		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
	}

	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		UME_PROFILE_FUNCTION();

		glViewport(x, y, width, height);
	}
}