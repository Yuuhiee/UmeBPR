#include "umepch.h"
#include "OpenGLShader.h"

#include <fstream>

#include "Ume/Renderer/Renderer.h"

#include <glm/gtc/type_ptr.hpp>

namespace Ume
{
	static GLenum ShaderTypeFromString(const std::string& type)
	{
		UME_PROFILE_FUNCTION();

		if (type == "vertex")	return GL_VERTEX_SHADER;
		if (type == "fragment")	return GL_FRAGMENT_SHADER;

		UME_CORE_ASSERT(false, "Unknown shader type!");
		return 0;
	}

	OpenGLShader::OpenGLShader(const std::string& filepath)
		: m_FilePath(filepath)
	{
		UME_PROFILE_FUNCTION();

		std::string& source = ReadFile(filepath);
		auto& shaderSources = Preprocess(source);
		Compile(shaderSources);

		// Get name
		auto lastSlash = filepath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filepath.rfind(".");
		lastDot = lastDot == std::string::npos ? filepath.size() : lastDot;
		m_Name = filepath.substr(lastSlash, lastDot - lastSlash);
	}

	OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
		: m_Name(name)
	{
		UME_PROFILE_FUNCTION();

		std::unordered_map<GLenum, std::string> shaderSources = {
			{GL_VERTEX_SHADER, vertexSrc},
			{GL_FRAGMENT_SHADER, fragmentSrc},
		};
		Compile(shaderSources);
	}

	OpenGLShader::~OpenGLShader()
	{
		UME_PROFILE_FUNCTION();

		glDeleteProgram(m_RendererID);
	}

	std::string OpenGLShader::ReadFile(const std::string& filepath)
	{
		UME_PROFILE_FUNCTION();

		std::string result;
		std::ifstream in(filepath, std::ios::in, std::ios::binary);
		bool open = in.is_open();
		if (in)
		{
			in.seekg(0, std::ios::end);
			result.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&result[0], result.size());
		}
		else
		{
			UME_CORE_ERROR("Failed to load file: {}", filepath);
		}

		return result;
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::Preprocess(const std::string& source)
	{
		UME_PROFILE_FUNCTION();

		std::unordered_map<GLenum, std::string> sourceMap;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);
		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos);
			UME_CORE_ASSERT(eol != std::string::npos, "Syntax error!");
			size_t begin = pos + typeTokenLength + 1;
			std::string type = source.substr(begin, eol - begin);
			UME_CORE_ASSERT(type == "vertex" || type == "fragment", "Invalid shader type!");

			size_t nextShaderPos = source.find_first_not_of("\r\n", eol);
			pos = source.find(typeToken, nextShaderPos);
			sourceMap[ShaderTypeFromString(type)] =
				source.substr(nextShaderPos, pos - (nextShaderPos == std::string::npos ? source.size() - 1 : nextShaderPos));
		}

		return sourceMap;
	}
	bool OpenGLShader::Compile(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		UME_PROFILE_FUNCTION();

		GLuint program = glCreateProgram();
		UME_CORE_ASSERT(shaderSources.size() <= 2, "Max supported shader quantity is 2!");
		std::array<GLuint, 2> shaderIDs;

		unsigned int index = 0;
		for (const auto& kv : shaderSources)
		{
			GLenum type = kv.first;
			const std::string& source = kv.second;

			GLuint shader = glCreateShader(type);

			const GLchar* sourceCode = source.c_str();
			glShaderSource(shader, 1, &sourceCode, 0);

			glCompileShader(shader);

			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

				glDeleteShader(shader);

				UME_CORE_ERROR("{0}", infoLog.data());
				//UME_CORE_ASSERT(false, "Shader compilation failure!");
				return false;
			}

			glAttachShader(program, shader);

			shaderIDs[index++] = shader;
		}

		glLinkProgram(program);

		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
			glDeleteProgram(program);

			for (index = 0; index < shaderSources.size(); index++)
				glDeleteShader(shaderIDs[index]);

			UME_CORE_ERROR("{0}", infoLog.data());
			UME_CORE_ASSERT(false, "Shader link failure!");
			return false;
		}

		for (index = 0; index < shaderSources.size(); index++)
			glDetachShader(program, shaderIDs[index]);

		if (m_RendererID)
			glDeleteProgram(m_RendererID);

		m_RendererID = program;
        return true;
	}

	GLint OpenGLShader::GetUniformLocation(const std::string& name)
	{
		UME_PROFILE_FUNCTION();

		if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
		{
			return m_UniformLocationCache[name];
		}

		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		if (location == -1)
		{
			UME_CORE_WARN("Uniform \"{}\" not found or have been optimized.", name);
			return location;
		}

		m_UniformLocationCache[name] = location;
		return location;
	}

	void OpenGLShader::BindImmediatly() const
	{
		glUseProgram(m_RendererID);
	}

	void OpenGLShader::UnbindImmediatly() const
	{
		glUseProgram(0);
	}

	void OpenGLShader::Bind() const
	{
		UME_PROFILE_FUNCTION();

		UME_RENDER_1(m_RendererID, {
			glUseProgram(m_RendererID);
		});
	}

	void OpenGLShader::Unbind() const
	{
		UME_PROFILE_FUNCTION();

		UME_RENDER({
			glUseProgram(0);
		});
	}

	void Ume::OpenGLShader::Reload()
	{
		std::string& source = ReadFile(m_FilePath);
		auto& shaderSources = Preprocess(source);
        if (Compile(shaderSources) && m_UniformToReload.Size > 0)
        {   
			BindImmediatly();
            // Upload the saved unformbuffer
            for (uint32_t i = 0; i < m_UniformToReload.Count; i++)
            {
                const UniformDecl &decl = m_UniformToReload.Uniforms[i];

                switch (decl.Type)
                {
                case UniformType::Int32:
                {
                    const std::string &name = decl.Name;
                    int32_t value = *(int32_t *)(m_UniformToReload.Buffer + decl.Offset);
                    SetInt(name, value);
                    break;
                }
                case UniformType::Float:
                {
                    const std::string &name = decl.Name;
                    float value = *(float *)(m_UniformToReload.Buffer + decl.Offset);
                    SetFloat(name, value);
                    break;
                }
                case UniformType::Float3:
                {
                    const std::string &name = decl.Name;
                    const glm::vec3 &value = *(glm::vec3 *)(m_UniformToReload.Buffer + decl.Offset);
                    SetFloat3(name, value);
                    break;
                }
                case UniformType::Float4:
                {
                    const std::string &name = decl.Name;
                    const glm::vec4 &value = *(glm::vec4 *)(m_UniformToReload.Buffer + decl.Offset);
                    SetFloat4(name, value);
                    break;
                }
                case UniformType::Matrix4:
                {
                    const std::string &name = decl.Name;
                    const glm::mat4 &value = *(glm::mat4 *)(m_UniformToReload.Buffer + decl.Offset);
                    SetMat4(name, value);
                    break;
                }
                }
            }
        }
	}

	void OpenGLShader::SetInt(const std::string& name, int value)
	{
		UME_PROFILE_FUNCTION();

		glUniform1i(GetUniformLocation(name), value);
	}

	void OpenGLShader::SetIntArray(const std::string& name, int* value, uint32_t count)
	{
		UME_PROFILE_FUNCTION();
		glUniform1iv (GetUniformLocation(name), count, value);
	}

	void OpenGLShader::SetFloat(const std::string& name, float value)
	{
		UME_PROFILE_FUNCTION();

		glUniform1f(GetUniformLocation(name), value);
	}

	void OpenGLShader::SetFloat2(const std::string& name, const glm::vec2& value)
	{
		UME_PROFILE_FUNCTION();

		glUniform2f(GetUniformLocation(name), value.x, value.y);
	}

	void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value)
	{
		UME_PROFILE_FUNCTION();

		glUniform3f(GetUniformLocation(name), value.x, value.y, value.z);
	}

	void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value)
	{
		UME_PROFILE_FUNCTION();

		glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w);
	}

    void OpenGLShader::SetFloat3Array(const std::string& name, const glm::vec3* value, uint32_t count)
    {
        UME_PROFILE_FUNCTION();

        glUniform3fv(GetUniformLocation(name), count, glm::value_ptr(value[0]));
    }

	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& matrix)
	{
		UME_PROFILE_FUNCTION();

		glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void Ume::OpenGLShader::SaveUniformBuffer(const UniformBufferBase& uniformBuffer)
	{
        m_UniformToReload.Buffer = new unsigned char[uniformBuffer.GetBufferSize()];
        memcpy(m_UniformToReload.Buffer, uniformBuffer.GetBuffer(), uniformBuffer.GetBufferSize());
        m_UniformToReload.Uniforms = new UniformDecl[uniformBuffer.GetUniformCount()];
        for (uint32_t i = 0; i < uniformBuffer.GetUniformCount(); i++)
        {
            m_UniformToReload.Uniforms[i] = uniformBuffer.GetUniforms()[i];
        }
        m_UniformToReload.Size = uniformBuffer.GetBufferSize();
        m_UniformToReload.Count = uniformBuffer.GetUniformCount();    
	}

	void OpenGLShader::UploadUniformBuffer(const UniformBufferBase& uniformBuffer)
	{
		for (uint32_t i = 0; i < uniformBuffer.GetUniformCount(); i++)
		{
			const UniformDecl& decl = uniformBuffer.GetUniforms()[i];
			
			switch (decl.Type)
			{
				case UniformType::Int32:
				{
					const std::string& name = decl.Name;
					int32_t value = *(int32_t*)(uniformBuffer.GetBuffer() + decl.Offset);
					UME_RENDER_S2(name, value, {
						self->SetInt(name, value);
					});
					break;
				}
				case UniformType::Float:
				{
					const std::string& name = decl.Name;
					float value = *(float*)(uniformBuffer.GetBuffer() + decl.Offset);
					UME_RENDER_S2(name, value, {
						self->SetFloat(name, value);
					});
					break;
				}
				case UniformType::Float3:
				{
					const std::string& name = decl.Name;
					const glm::vec3& value = *(glm::vec3*)(uniformBuffer.GetBuffer() + decl.Offset);
					UME_RENDER_S2(name, value, {
						self->SetFloat3(name, value);
					});
					break;
				}
				case UniformType::Float4:
				{
					const std::string& name = decl.Name;
					const glm::vec4& value = *(glm::vec4*)(uniformBuffer.GetBuffer() + decl.Offset);
					UME_RENDER_S2(name, value, {
						self->SetFloat4(name, value);
					});
					break;
				}
				case UniformType::Matrix4:
				{
					const std::string& name = decl.Name;
					const glm::mat4& value = *(glm::mat4*)(uniformBuffer.GetBuffer() + decl.Offset);
					UME_RENDER_S2(name, value, {
						self->SetMat4(name, value);
					});
					break;
				}
			}
		}
	}
	void Ume::OpenGLShader::UploadUniformBufferImmediatly(const UniformBufferBase& uniformBuffer)
	{
		for (uint32_t i = 0; i < uniformBuffer.GetUniformCount(); i++)
		{
			const UniformDecl& decl = uniformBuffer.GetUniforms()[i];

			switch (decl.Type)
			{
			case UniformType::Int32:
			{
				const std::string& name = decl.Name;
				int32_t value = *(int32_t*)(uniformBuffer.GetBuffer() + decl.Offset);
				SetInt(name, value);
				break;
			}
			case UniformType::Float:
			{
				const std::string& name = decl.Name;
				float value = *(float*)(uniformBuffer.GetBuffer() + decl.Offset);
				SetFloat(name, value);
				break;
			}
			case UniformType::Float3:
			{
				const std::string& name = decl.Name;
				const glm::vec3& value = *(glm::vec3*)(uniformBuffer.GetBuffer() + decl.Offset);
				SetFloat3(name, value);
				break;
			}
			case UniformType::Float4:
			{
				const std::string& name = decl.Name;
				const glm::vec4& value = *(glm::vec4*)(uniformBuffer.GetBuffer() + decl.Offset);
				SetFloat4(name, value);
				break;
			}
			case UniformType::Matrix4:
			{
				const std::string& name = decl.Name;
				const glm::mat4& value = *(glm::mat4*)(uniformBuffer.GetBuffer() + decl.Offset);
				SetMat4(name, value);
				break;
			}
			}
		}
	}
}