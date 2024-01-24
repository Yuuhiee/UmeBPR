#pragma once

#include "Ume/Renderer/Texture.h"
namespace Ume
{
    class OpenGLTexture2D : public Texture2D
    {
    public:
        OpenGLTexture2D(uint32_t width, uint32_t height, const TextureSpecification &specification);
        OpenGLTexture2D(const std::string &path, const TextureSpecification &specification);
        ~OpenGLTexture2D();

        inline virtual uint32_t GetWidth() const override { return m_Width; }
        inline virtual uint32_t GetHeight() const override { return m_Height; }

        virtual void SetData(void *data, uint32_t size) override;
        
        virtual uint32_t GetRendererID() const override { return m_RendererID; }
        virtual const std::string& GetFilePath() override { return m_Path; }

        virtual void Bind(int slot = 0) const override;
        virtual bool operator==(const Texture &other) const override
        {
            return ((OpenGLTexture2D &)other).m_RendererID == m_RendererID;
        }

    private:
        std::string m_Path;
        uint32_t m_Width, m_Height;
        uint32_t m_RendererID;
        TextureSpecification m_Specification;
        int m_Format, m_InternalFormat;
    };

    class OpenGLTextureCube : public TextureCube
    {
    public:
        OpenGLTextureCube(const std::string &path, const TextureSpecification &specification);
        OpenGLTextureCube(const std::string &directory, const std::array<std::string, 6> &faces, const std::string &ext, const TextureSpecification &specification);
        ~OpenGLTextureCube();

        inline virtual uint32_t GetWidth() const override { return m_Width; }
        inline virtual uint32_t GetHeight() const override { return m_Height; }

        virtual void SetData(void *data, uint32_t size) override {}

        virtual uint32_t GetRendererID() const override { return m_RendererID; }
        virtual const std::string& GetFilePath() override { return m_Path; }

        virtual void Bind(int slot = 0) const override;
        virtual bool operator==(const Texture &other) const override
        {
            return ((OpenGLTextureCube &)other).m_RendererID == m_RendererID;
        }

    private:
        std::string m_Path;
        uint32_t m_Width, m_Height;
        uint32_t m_RendererID;
        TextureSpecification m_Specification;
        int m_Format, m_InternalFormat;
        unsigned char *m_ImageData;
    };
}
