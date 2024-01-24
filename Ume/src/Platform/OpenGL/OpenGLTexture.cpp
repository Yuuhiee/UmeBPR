#include "umepch.h"
#include "OpenGLTexture.h"

#include "glad/glad.h"
#include "stb_image.h"

#include "Ume/Renderer/Renderer.h"

namespace Ume
{
    void Texture2D::BindByID(uint32_t rendererID, int slot)
    {
        UME_RENDER_2(slot, rendererID, {
            glActiveTexture(GL_TEXTURE0 + slot);
            glBindTexture(GL_TEXTURE_2D, rendererID);
        });
    }
    
    static unsigned char *UmeLoadImage(const std::string &path, int &width, int &height, int &channels, bool flip)
    {
        stbi_set_flip_vertically_on_load(flip);
        unsigned char *data = stbi_load(path.c_str(), &width, &height, &channels, 0);
        UME_CORE_ASSERT(data, "Failed to load image!");
        return data;
    }

    static std::pair<GLenum, GLenum> UmeToOpenGLTextureFormat(ImageFormat format)
    {
        switch (format)
        {
        case ImageFormat::RED8UI:
            return {GL_RED, GL_RED};
        case ImageFormat::RGB:
            return {GL_RGB8, GL_RGB};
        case ImageFormat::RGB16F:
            return { GL_RGB16F, GL_RGB };
        case ImageFormat::RGBA:
            return {GL_RGBA8, GL_RGBA};
        }
        UME_CORE_ERROR("Error TextureFormat!");
        return {0, 0};
    }

    static std::pair<GLenum, GLenum> OpenGLTextureFormatFromChannel(int channel)
    {
        switch (channel)
        {
        case 1:
            return {GL_RED, GL_RED};
        case 3:
            return {GL_RGB8, GL_RGB};
        case 4:
            return {GL_RGBA8, GL_RGBA};
        }
        UME_CORE_ERROR("Error TextureFormat!");
        return {0, 0};
    }

    static GLenum UmeToOpenGLTextureFilter(TextureFilter filter)
    {
        switch (filter)
        {
        case Ume::TextureFilter::Nearest:
            return GL_NEAREST;
        case Ume::TextureFilter::Linear:
            return GL_LINEAR;
        case Ume::TextureFilter::MipMapLinear:
            return GL_LINEAR_MIPMAP_LINEAR;
        }
        UME_CORE_ERROR("Error TextureFilter!");
        return 0;
    }

    static GLenum UmeToOpenGLTextureWrap(TextureWrap wrap)
    {
        switch (wrap)
        {
        case TextureWrap::Clamp:
            return GL_CLAMP_TO_EDGE;
        case TextureWrap::Repeat:
            return GL_REPEAT;
        }
        UME_CORE_ERROR("Error TextureWrap!");
        return 0;
    }

    OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height, const TextureSpecification &specification)
    {
        TextureSpecification::Copy(&m_Specification, specification);

        m_Width = (uint32_t)width;
        m_Height = (uint32_t)height;

        auto [internalFormat, format] = UmeToOpenGLTextureFormat(specification.Format);

        m_Format = format;
        m_InternalFormat = internalFormat;

        /*switch (channels)
        {
            case 1: internalFormat = GL_RED; format = GL_RED;  break;
            case 3: internalFormat = GL_RGB8; format = GL_RGB;	 break;
            case 4: internalFormat = GL_RGBA8; format = GL_RGBA; break;
            default: UME_CORE_ERROR("Unsupported texture format!");
        }*/

        GLenum filter = UmeToOpenGLTextureFilter(specification.Filter);
        GLenum wrap = UmeToOpenGLTextureWrap(specification.Wrap);

        GLenum type = specification.Format == ImageFormat::RGB16F ? GL_FLOAT : GL_UNSIGNED_BYTE;
        glGenTextures(1, &m_RendererID);
        glBindTexture(GL_TEXTURE_2D, m_RendererID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, (int)width, (int)height, 0, format, type, nullptr);

        if (specification.GenMips)
            glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
    }

    OpenGLTexture2D::OpenGLTexture2D(const std::string &path, const TextureSpecification &specification)
        : m_Path(path)
    {
        // get ext name
        std::string ext = path.substr(path.find_last_of(".") + 1);

        int width, height, channels;
        stbi_uc *data = UmeLoadImage(path, width, height, channels, true);
        m_Width = (uint32_t)width;
        m_Height = (uint32_t)height;

#if 0
        if (false)
        {
            uint32_t faceWidth = m_Width / 4;
            uint32_t faceHeight = m_Height / 3;

            std::array<stbi_uc *, 6> faces;
            for (size_t i = 0; i < faces.size(); i++)
                faces[i] = new stbi_uc[faceWidth * faceHeight * 3]; // 3 BPP

            int faceIndex = 0;

            for (size_t i = 0; i < 4; i++)
            {
                for (size_t y = 0; y < faceHeight; y++)
                {
                    size_t yOffset = y + faceHeight;
                    for (size_t x = 0; x < faceWidth; x++)
                    {
                        size_t xOffset = x + i * faceWidth;
                        faces[faceIndex][(x + y * faceWidth) * 3 + 0] = data[(xOffset + yOffset * m_Width) * 3 + 0];
                        faces[faceIndex][(x + y * faceWidth) * 3 + 1] = data[(xOffset + yOffset * m_Width) * 3 + 1];
                        faces[faceIndex][(x + y * faceWidth) * 3 + 2] = data[(xOffset + yOffset * m_Width) * 3 + 2];
                    }
                }
                faceIndex++;
            }

            for (size_t i = 1; i < 3; i++)
            {
                for (size_t y = 0; y < faceHeight; y++)
                {
                    size_t yOffset = y + i * faceHeight;
                    for (size_t x = 0; x < faceWidth; x++)
                    {
                        size_t xOffset = x + faceWidth;
                        faces[faceIndex][(x + y * faceWidth) * 3 + 0] = data[(xOffset + yOffset * m_Width) * 3 + 0];
                        faces[faceIndex][(x + y * faceWidth) * 3 + 1] = data[(xOffset + yOffset * m_Width) * 3 + 1];
                        faces[faceIndex][(x + y * faceWidth) * 3 + 2] = data[(xOffset + yOffset * m_Width) * 3 + 2];
                    }
                }
                faceIndex++;
            }

            width = faceWidth;
            height = faceHeight;
            data = faces[5];
        }
#endif
        TextureSpecification::Copy(&m_Specification, specification);

        GLenum format = 0, internalFormat = 0;

        // switch (specification.Format)
        //{
        // case ImageFormat::RED8UI: internalFormat = GL_RED; format = GL_RED;  break;
        // case ImageFormat::RGB: internalFormat = GL_RGB8; format = GL_RGB;	 break;
        // case ImageFormat::RGBA: internalFormat = GL_RGBA8; format = GL_RGBA; break;
        // default: UME_CORE_ERROR("Unsupported texture format!");
        // }

        if (specification.SRGB)
        {
            switch (channels)
            {
            case 3:
                internalFormat = GL_SRGB8;
                format = GL_RGB;
                break;
            case 4:
                internalFormat = GL_SRGB8_ALPHA8;
                format = GL_RGBA;
                break;
            default:
                UME_CORE_ERROR("Unsupported texture format!");
            }
        }
        else
        {
            // use function OpenGLTextureFormatFromChannel
            auto &pair = OpenGLTextureFormatFromChannel(channels);
            internalFormat = pair.first;
            format = pair.second;
        }

        m_Format = format;
        m_InternalFormat = internalFormat;

        GLenum filter = UmeToOpenGLTextureFilter(specification.Filter);
        GLenum wrap = UmeToOpenGLTextureWrap(specification.Wrap);

        glGenTextures(1, &m_RendererID);
        glBindTexture(GL_TEXTURE_2D, m_RendererID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        // glTexStorage2D(GL_TEXTURE_2D, 0, internalFormat, width, height);
        // glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);

        if (specification.GenMips)
            glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

        if (specification.Anisotropy)
        {
            // glTextureParameterf(self->m_RendererID, GL_TEXTURE_MAX_ANISOTROPY, RendererAPI::GetCapabilities().MaxAnisotropy);
        }

        stbi_image_free(data);
    }

    OpenGLTexture2D::~OpenGLTexture2D()
    {
        glDeleteTextures(1, &m_RendererID);
    }

    void OpenGLTexture2D::SetData(void *data, uint32_t size)
    {
        glBindTexture(GL_TEXTURE_2D, m_RendererID);
        GLenum type = m_Specification.Format == ImageFormat::RGB16F ? GL_FLOAT : GL_UNSIGNED_BYTE;
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Width, m_Height, m_Format, type, data);
    }

    void OpenGLTexture2D::Bind(int slot) const
    {
        UME_RENDER_2(slot, m_RendererID, {
            glActiveTexture(GL_TEXTURE0 + slot);
            glBindTexture(GL_TEXTURE_2D, m_RendererID);
        });
    }

    OpenGLTextureCube::OpenGLTextureCube(const std::string &path, const TextureSpecification &specification)
    {
        TextureSpecification::Copy(&m_Specification, specification);

        int width, height, channels;
        m_ImageData = UmeLoadImage(path, width, height, channels, false);

        m_Width = width;
        m_Height = height;

        uint32_t faceWidth = m_Width / 4;
        uint32_t faceHeight = m_Height / 3;

        std::array<stbi_uc *, 6> faces;
        for (size_t i = 0; i < faces.size(); i++)
            faces[i] = new stbi_uc[faceWidth * faceHeight * 3]; // 3 BPP

        int faceIndex = 0;

        for (size_t i = 0; i < 4; i++)
        {
            for (size_t y = 0; y < faceHeight; y++)
            {
                size_t yOffset = y + faceHeight;
                for (size_t x = 0; x < faceWidth; x++)
                {
                    size_t xOffset = x + i * faceWidth;
                    faces[faceIndex][(x + y * faceWidth) * 3 + 0] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 0];
                    faces[faceIndex][(x + y * faceWidth) * 3 + 1] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 1];
                    faces[faceIndex][(x + y * faceWidth) * 3 + 2] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 2];
                }
            }
            faceIndex++;
        }

        for (size_t i = 0; i < 3; i++)
        {
            // Skip the middle one
            if (i == 1)
                continue;

            for (size_t y = 0; y < faceHeight; y++)
            {
                size_t yOffset = y + i * faceHeight;
                for (size_t x = 0; x < faceWidth; x++)
                {
                    size_t xOffset = x + faceWidth;
                    faces[faceIndex][(x + y * faceWidth) * 3 + 0] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 0];
                    faces[faceIndex][(x + y * faceWidth) * 3 + 1] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 1];
                    faces[faceIndex][(x + y * faceWidth) * 3 + 2] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 2];
                }
            }
            faceIndex++;
        }

        auto [internalFormat, format] = OpenGLTextureFormatFromChannel(/*self->*/ channels);
        m_InternalFormat = internalFormat;
        m_Format = format;

        // glGenTextures(1, &m_RendererID);
        // glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

        // stbi_set_flip_vertically_on_load(false);

        // for (unsigned int i = 0; i < faces.size(); i++)
        // {
        //     glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, faceWidth, faceHeight,
        //                  0, GL_RGB, GL_UNSIGNED_BYTE, faces[i]);
        // }

        // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // stbi_image_free(m_ImageData);

        // glGenTextures(1, &m_RendererID);
        // glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

        // for (size_t i = 0; i < faces.size(); i++)
        // {
        //     glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[i]);
        // }

        // GLenum filter = UmeToOpenGLTextureFilter(specification.Filter);
        // GLenum wrap = UmeToOpenGLTextureWrap(specification.Wrap);

        // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, filter);
        // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, filter);

        // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrap);
        // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrap);
        // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrap);

        // if (specification.GenMips)
        //     glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        // for (size_t i = 0; i < faces.size(); i++)
        //     delete[] faces[i];

        // stbi_image_free(m_ImageData);

        // UME_RENDER_S3(faces, faceWidth, faceHeight, {
        glGenTextures(1, &m_RendererID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

        auto filter = UmeToOpenGLTextureFilter(specification.Filter);
        auto wrap = UmeToOpenGLTextureWrap(specification.Wrap);

        // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, filter);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrap);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrap);
        // glTextureParameterf(self->m_RendererID, GL_TEXTURE_MAX_ANISOTROPY, RendererAPI::GetCapabilities().MaxAnisotropy);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[2]);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[0]);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[4]);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[5]);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[1]);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[3]);

        if (specification.GenMips)
            glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        glBindTexture(GL_TEXTURE_2D, 0);

        for (size_t i = 0; i < faces.size(); i++)
            delete[] faces[i];

        stbi_image_free(m_ImageData);
        //});
    }

    OpenGLTextureCube::OpenGLTextureCube(const std::string &directory, const std::array<std::string, 6> &faces, const std::string &ext, const TextureSpecification &specification)
    {
        TextureSpecification::Copy(&m_Specification, specification);

        glGenTextures(1, &m_RendererID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

        int width, height, channel;
        for (unsigned int i = 0; i < faces.size(); i++)
        {
            std::string path = directory + "/" + faces[i] + "." + ext;
            unsigned char *data = UmeLoadImage(path, width, height, channel, false);
            if (data)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height,
                             0, GL_RGB, GL_UNSIGNED_BYTE, data);
                stbi_image_free(data);
            }
            else
            {
                UME_CORE_ERROR("Cubemap texture failed to load at path: {0}", path);
                stbi_image_free(data);
            }
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    OpenGLTextureCube::~OpenGLTextureCube()
    {
        glDeleteTextures(1, &m_RendererID);
    }

    void OpenGLTextureCube::Bind(int slot) const
    {
        UME_RENDER_2(slot, m_RendererID, {
            glActiveTexture(GL_TEXTURE0 + slot);
            glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
        });
    }
}