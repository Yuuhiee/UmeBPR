#pragma once

#include "Ume/Core/Base.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"


typedef unsigned char byte;

namespace Ume
{
	enum class UniformType
	{
		None = 0,
		Float, Float2, Float3, Float4, Float3Array,
		Matrix3, Matrix4,
		Int32, Uint32,
		Bool,
	};

	struct UniformDecl
	{
		UniformType Type;
		std::ptrdiff_t Offset;
		std::string Name;
	};

	struct UniformBuffer
	{
		byte* Buffer;
		std::vector<UniformDecl> Uniforms;
	};

	struct UniformBufferBase
	{
		virtual const byte* GetBuffer() const = 0;
		virtual const UniformDecl* GetUniforms() const = 0;
		virtual uint32_t GetUniformCount() const = 0;
        virtual uint32_t GetBufferSize() const = 0;
	};

	template<uint32_t Size, uint32_t Count>
	struct UniformBufferDeclaration : public UniformBufferBase
	{
		byte Buffer[Size];
		UniformDecl Uniforms[Count];
		std::ptrdiff_t Cursor = 0;
		uint32_t Index = 0;

		virtual const byte* GetBuffer() const override { return Buffer; }
		virtual const UniformDecl* GetUniforms() const override { return Uniforms; }
		virtual uint32_t GetUniformCount() const override { return Count; }
        virtual uint32_t GetBufferSize() const override { return Size; }

		template<typename T>
		void Push(const std::string& name, const T& data) {}

		template<>
		void Push(const std::string& name, const int32_t& data)
		{
			Uniforms[Index++] = { UniformType::Int32, Cursor, name };
			memcpy(Buffer + Cursor, &data, sizeof(int32_t));
			Cursor += sizeof(int32_t);
		}
		template<>
		void Push(const std::string& name, const float& data)
		{
			Uniforms[Index++] = { UniformType::Float, Cursor, name };
			memcpy(Buffer + Cursor, &data, sizeof(float));
			Cursor += sizeof(float);
		}
		template<>
		void Push(const std::string& name, const glm::vec3& data)
		{
			Uniforms[Index++] = { UniformType::Float3, Cursor, name };
			memcpy(Buffer + Cursor, glm::value_ptr(data), sizeof(glm::vec3));
			Cursor += sizeof(glm::vec3);
		}
		template<>
		void Push(const std::string& name, const glm::vec4& data)
		{
			Uniforms[Index++] = { UniformType::Float4, Cursor, name };
			memcpy(Buffer + Cursor, glm::value_ptr(data), sizeof(glm::vec4));
			Cursor += sizeof(glm::vec4);
		}
		template<>
		void Push(const std::string& name, const glm::mat4& data)
		{
			Uniforms[Index++] = { UniformType::Matrix4, Cursor, name };
			memcpy(Buffer + Cursor, glm::value_ptr(data), sizeof(glm::mat4));
			Cursor += sizeof(glm::mat4);
		}
	};

	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual void BindImmediatly() const = 0;
		virtual void UnbindImmediatly() const = 0;

		virtual void Reload() = 0;

		virtual void SetInt(const std::string& name, int value) = 0;
		virtual void SetIntArray(const std::string& name, int* value, uint32_t count) = 0;
		virtual void SetFloat(const std::string& name, float value) = 0;
		virtual void SetFloat2(const std::string& name, const glm::vec2& value) = 0;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) = 0;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) = 0;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;
        virtual void SetFloat3Array(const std::string& name, const glm::vec3* value, uint32_t count) = 0;

		virtual void SaveUniformBuffer(const UniformBufferBase &uniformBuffer) = 0;
        virtual void UploadUniformBuffer(const UniformBufferBase &uniformBuffer) = 0;
        virtual void UploadUniformBufferImmediatly(const UniformBufferBase& uniformBuffer) = 0;

		virtual uint32_t GetRendererID() const = 0;
		virtual const std::string& GetName() const = 0;

		static Ref<Shader> Create(const std::string& filepath);
		static Ref<Shader> Create(const std::string& name,const std::string& vertexSrc, const std::string& fragmentSrc);
	};

	class ShaderLibrary
	{
	public:
		void Add(const Ref<Shader>& shader);
		void Add(const std::string& name, const Ref<Shader>& shader);
		void Load(const std::string& filepath);
		void Load(const std::string& name, const std::string& filepath);
		const Ref<Shader>& Get(const std::string& name);
		inline const std::unordered_map<std::string, Ref<Shader>>& GetShaders() const { return m_Shaders; }

		inline bool Exists(const std::string& name) const { return m_Shaders.find(name) != m_Shaders.end(); }
	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	};
}