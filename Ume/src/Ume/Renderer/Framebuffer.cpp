#include "umepch.h"
#include "Framebuffer.h"

#include "Ume/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"

namespace Ume
{
	Framebuffer* Framebuffer::Create(uint32_t width, uint32_t height, FramebufferFormat format)
	{
		UME_PROFILE_FUNCTION();

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			UME_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			//return CreateRef<OpenGLFramebuffer>(width, height, format);
			auto frambuffer = new OpenGLFramebuffer(width, height, format);
			FramebufferPool::Get()->Add(frambuffer);
			return frambuffer;
		}

		UME_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
	}

	Ref<FramebufferPool> FramebufferPool::s_Instance = CreateRef<FramebufferPool>();
	FramebufferPool::FramebufferPool(uint32_t capcity)
	{
	}

	FramebufferPool::~FramebufferPool()
	{
	}

	std::weak_ptr<Framebuffer> FramebufferPool::AllocateBuffer()
	{
		return std::weak_ptr<Framebuffer>();
	}

	void FramebufferPool::Add(Framebuffer* framebuffer)
	{
		m_Pool.push_back(framebuffer);
	}
}