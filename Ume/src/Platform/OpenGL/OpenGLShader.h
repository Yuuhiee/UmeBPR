#pragma once

#include "glm/glm.hpp"
#include "glad/glad.h"

#include "Ume/Renderer/Shader.h"

namespace Ume
{
	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& filepath);
		OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
		virtual ~OpenGLShader() override;

		virtual void Bind() const override;
		virtual void Unbind() const override;
		virtual void BindImmediatly() const override;
		virtual void UnbindImmediatly() const override;

		virtual void Reload() override;
		virtual void SetInt(const std::string& name, int value) override;
		virtual void SetIntArray(const std::string& name, int* value, uint32_t count) override;
		virtual void SetFloat(const std::string& name, float value) override;
		virtual void SetFloat2(const std::string& name, const glm::vec2& value) override;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) override;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) override;
        virtual void SetFloat3Array(const std::string& name, const glm::vec3* value, uint32_t count) override;

		virtual void SetMat4(const std::string& name, const glm::mat4& value) override;

		virtual void SaveUniformBuffer(const UniformBufferBase& uniformBuffer) override;
		virtual void UploadUniformBuffer(const UniformBufferBase& uniformBuffer) override;
		virtual void UploadUniformBufferImmediatly(const UniformBufferBase& uniformBuffer) override;

		inline virtual const std::string& GetName() const override { return m_Name; }
		virtual inline uint32_t GetRendererID() const override { return m_RendererID; }
    private:
        uint32_t m_RendererID = 0;
		std::string m_FilePath;
		std::string m_Name;
		std::unordered_map<std::string, GLuint> m_UniformLocationCache;
        struct UniformToReload
        {
            unsigned char* Buffer = nullptr;
            UniformDecl* Uniforms = nullptr;
            uint32_t Size = 0;
            uint32_t Count = 0;
        };
        UniformToReload m_UniformToReload;
	private:
		std::string ReadFile(const std::string& filepath);
		std::unordered_map<GLenum, std::string> Preprocess(const std::string& source);
		bool Compile(const std::unordered_map<GLenum, std::string>& shaderSources);
		GLint GetUniformLocation(const std::string& name);
	};
}