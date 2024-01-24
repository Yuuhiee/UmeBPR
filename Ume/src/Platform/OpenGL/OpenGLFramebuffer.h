#pragma once

#include "Ume/Renderer/Framebuffer.h"
namespace Ume
{
	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(uint32_t width, uint32_t height, FramebufferFormat format);
		~OpenGLFramebuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual void BindTexture(uint32_t slot = 0) const override;

		inline virtual uint32_t GetRendererID() const override { return m_RendererID; }
		inline virtual uint32_t GetColorAttachmentRendererID() const override { return m_ColorAttachment; }
		inline virtual uint32_t GetDepthAttachmentRendererID() const override { return m_DepthAttachment; }

		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }
		FramebufferFormat GetFormat() const { return m_Format; }
	private:
		uint32_t m_RendererID = 0;
		uint32_t m_ColorAttachment = 0, m_DepthAttachment = 0;
		uint32_t m_Width, m_Height;
		FramebufferFormat m_Format;
	};
}
