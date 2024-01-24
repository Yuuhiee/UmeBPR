#pragma once

namespace Ume
{
	enum class FramebufferFormat
	{
		None = 0,
		RGBA8,
		RGBA16F
	};

	class Framebuffer
	{
	public:
		virtual ~Framebuffer() = default;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void BindTexture(uint32_t slot = 0) const = 0;

		virtual uint32_t GetRendererID() const = 0;
		virtual uint32_t GetColorAttachmentRendererID() const = 0;
		virtual uint32_t GetDepthAttachmentRendererID() const = 0;
	public:
		static Framebuffer* Create(uint32_t width, uint32_t height, FramebufferFormat format);
	};

	class FramebufferPool
	{
	public:
		FramebufferPool(uint32_t capcity = 32);
		~FramebufferPool();

		std::weak_ptr<Framebuffer> AllocateBuffer();
		void Add(Framebuffer* framebuffer);

		inline std::vector<Framebuffer*>& GetPool() { return m_Pool; }

		inline static const Ref<FramebufferPool>& Get() { return s_Instance; }
	private:
		std::vector<Framebuffer*> m_Pool;
		static Ref<FramebufferPool> s_Instance;
	};
}
