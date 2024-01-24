#include "umepch.h"
#include "RenderCommandQueue.h"

namespace Ume
{
	RenderCommandQueue::RenderCommandQueue(uint32_t size)
	{
		m_CommandBuffer = new unsigned char[size];
		m_CommandBufferPtr = m_CommandBuffer;
		memset(m_CommandBuffer, 0, size);
	}

	RenderCommandQueue::~RenderCommandQueue()
	{
		delete[] m_CommandBuffer;
	}

	void* RenderCommandQueue::Allocate(RenderCommandFn func, uint32_t size)
	{
		*(RenderCommandFn*)m_CommandBufferPtr = func;
		m_CommandBufferPtr += sizeof(RenderCommandFn);

		*(int*)m_CommandBufferPtr = size;
		m_CommandBufferPtr += sizeof(uint32_t);

		void* memory = m_CommandBufferPtr;
		m_CommandBufferPtr += size;

		m_CommandCount++;

		return memory;
	}

	void RenderCommandQueue::Execute()
	{
		//UME_CORE_TRACE("RenderCommandQueue::Execute -- {0} commands, {1} bytes", m_CommandCount, (m_CommandBufferPtr - m_CommandBuffer));

		unsigned char* buffer = m_CommandBuffer;

		for (uint32_t i = 0; i < m_CommandCount; i++)
		{
			RenderCommandFn func = *(RenderCommandFn*)buffer;
			buffer += sizeof(RenderCommandFn);


			uint32_t size = *(uint32_t*)buffer;
			buffer += sizeof(uint32_t);
			func(buffer);
			buffer += size;
		}

		m_CommandBufferPtr = m_CommandBuffer;
		m_CommandCount = 0;
	}
}
