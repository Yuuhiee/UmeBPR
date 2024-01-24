#pragma once

#include "RendererAPI.h"
#include "VertexArray.h"
#include "RenderCommandQueue.h"
#include "Camera.h"
#include "Shader.h"

namespace Ume
{
	class Renderer
	{
	public:
		typedef void(*RenderCommandFn)(void*);

		static void Init();
		static void StartScene(const Camera& camera);
		static void EndScene();
		static void Submit(const Ref<VertexArray>& vertexArray, bool depthTest = true);
		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
		inline static const Ref<ShaderLibrary>& GetShaderLibrary() { return s_ShaderLibrary; }
	public:
		static void OnWindowResize(uint32_t width, uint32_t height);
	public:
		static RenderCommandQueue& GetCommandQueue() { return s_CommandQueue; }
        static void Render() { s_CommandQueue.Execute(); }
		static void* Allocate(RenderCommandFn func, uint32_t size) { return s_CommandQueue.Allocate(func, size); }
    public:
        static void Clear();
        static void SetClearColor(const glm::vec4& color);
	private:
		struct SceneData
		{
			glm::mat4 ViewProjection;
		};
		static Scope<SceneData> s_SceneData;
        static Scope<RendererAPI> s_RendererAPI;
        static Ref<ShaderLibrary> s_ShaderLibrary;
		static RenderCommandQueue s_CommandQueue;
	};
}

#define UME_RENDER_UNIQUE(x) x ## __LINE__

#define UME_RENDER(code) \
	struct UME_RENDER_UNIQUE(RenderCommand) \
    {\
        static void Execute(void*)\
        {\
            code\
        }\
    };\
	{\
		auto mem = ::Ume::Renderer::Allocate(UME_RENDER_UNIQUE(RenderCommand)::Execute, sizeof(UME_RENDER_UNIQUE(RenderCommand)));\
		new (mem) UME_RENDER_UNIQUE(RenderCommand)();\
	}\

#define UME_RENDER_1(arg0, code) \
	do {\
    struct UME_RENDER_UNIQUE(RenderCommand) \
    {\
		UME_RENDER_UNIQUE(RenderCommand)(typename ::std::remove_const<typename ::std::remove_reference<decltype(arg0)>::type>::type arg0) \
		: arg0(arg0) {}\
		\
        static void Execute(void* argBuffer)\
        {\
			auto& arg0 = ((UME_RENDER_UNIQUE(RenderCommand)*)argBuffer)->arg0;\
            code\
        }\
		\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg0)>::type>::type arg0;\
    };\
	{\
		auto mem = ::Ume::Renderer::Allocate(UME_RENDER_UNIQUE(RenderCommand)::Execute, sizeof(UME_RENDER_UNIQUE(RenderCommand)));\
		new (mem) UME_RENDER_UNIQUE(RenderCommand)(arg0);\
	} } while(0)

#define UME_RENDER_2(arg0, arg1, code) \
    struct UME_RENDER_UNIQUE(RenderCommand) \
    {\
		UME_RENDER_UNIQUE(RenderCommand)(typename ::std::remove_const<typename ::std::remove_reference<decltype(arg0)>::type>::type arg0,\
											typename ::std::remove_const<typename ::std::remove_reference<decltype(arg1)>::type>::type arg1) \
		: arg0(arg0), arg1(arg1) {}\
		\
        static void Execute(void* argBuffer)\
        {\
			auto& arg0 = ((UME_RENDER_UNIQUE(RenderCommand)*)argBuffer)->arg0;\
			auto& arg1 = ((UME_RENDER_UNIQUE(RenderCommand)*)argBuffer)->arg1;\
            code\
        }\
		\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg0)>::type>::type arg0;\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg1)>::type>::type arg1;\
    };\
	{\
		auto mem = ::Ume::Renderer::Allocate(UME_RENDER_UNIQUE(RenderCommand)::Execute, sizeof(UME_RENDER_UNIQUE(RenderCommand)));\
		new (mem) UME_RENDER_UNIQUE(RenderCommand)(arg0, arg1);\
	}\

#define UME_RENDER_3(arg0, arg1, arg2, code) \
    struct UME_RENDER_UNIQUE(RenderCommand) \
    {\
		UME_RENDER_UNIQUE(RenderCommand)(typename ::std::remove_const<typename ::std::remove_reference<decltype(arg0)>::type>::type arg0,\
											typename ::std::remove_const<typename ::std::remove_reference<decltype(arg1)>::type>::type arg1,\
											typename ::std::remove_const<typename ::std::remove_reference<decltype(arg2)>::type>::type arg2) \
		: arg0(arg0), arg1(arg1), arg2(arg2) {}\
		\
        static void Execute(void* argBuffer)\
        {\
			auto& arg0 = ((UME_RENDER_UNIQUE(RenderCommand)*)argBuffer)->arg0;\
			auto& arg1 = ((UME_RENDER_UNIQUE(RenderCommand)*)argBuffer)->arg1;\
			auto& arg2 = ((UME_RENDER_UNIQUE(RenderCommand)*)argBuffer)->arg2;\
            code\
        }\
		\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg0)>::type>::type arg0;\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg1)>::type>::type arg1;\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg2)>::type>::type arg2;\
    };\
	{\
		auto mem = ::Ume::Renderer::Allocate(UME_RENDER_UNIQUE(RenderCommand)::Execute, sizeof(UME_RENDER_UNIQUE(RenderCommand)));\
		new (mem) UME_RENDER_UNIQUE(RenderCommand)(arg0, arg1, arg2);\
	}\

#define UME_RENDER_4(arg0, arg1, arg2, arg3, code) \
    struct UME_RENDER_UNIQUE(RenderCommand) \
    {\
		UME_RENDER_UNIQUE(RenderCommand)(typename ::std::remove_const<typename ::std::remove_reference<decltype(arg0)>::type>::type arg0,\
											typename ::std::remove_const<typename ::std::remove_reference<decltype(arg1)>::type>::type arg1,\
											typename ::std::remove_const<typename ::std::remove_reference<decltype(arg2)>::type>::type arg2,\
											typename ::std::remove_const<typename ::std::remove_reference<decltype(arg3)>::type>::type arg3)\
		: arg0(arg0), arg1(arg1), arg2(arg2), arg3(arg3) {}\
		\
        static void Execute(void* argBuffer)\
        {\
			auto& arg0 = ((UME_RENDER_UNIQUE(RenderCommand)*)argBuffer)->arg0;\
			auto& arg1 = ((UME_RENDER_UNIQUE(RenderCommand)*)argBuffer)->arg1;\
			auto& arg2 = ((UME_RENDER_UNIQUE(RenderCommand)*)argBuffer)->arg2;\
			auto& arg3 = ((UME_RENDER_UNIQUE(RenderCommand)*)argBuffer)->arg3;\
            code\
        }\
		\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg0)>::type>::type arg0;\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg1)>::type>::type arg1;\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg2)>::type>::type arg2;\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg3)>::type>::type arg3;\
    };\
	{\
		auto mem = Renderer::Allocate(UME_RENDER_UNIQUE(RenderCommand)::Execute, sizeof(UME_RENDER_UNIQUE(RenderCommand)));\
		new (mem) UME_RENDER_UNIQUE(RenderCommand)(arg0, arg1, arg2, arg3);\
	}

#define UME_RENDER_S(code) auto self = this;\
	UME_RENDER_1(self, code)

#define UME_RENDER_S1(arg0, code) auto self = this;\
	UME_RENDER_2(self, arg0, code)

#define UME_RENDER_S2(arg0, arg1, code) auto self = this;\
	UME_RENDER_3(self, arg0, arg1, code)

#define UME_RENDER_S3(arg0, arg1, arg2, code) auto self = this;\
	UME_RENDER_4(self, arg0, arg1, arg2, code)