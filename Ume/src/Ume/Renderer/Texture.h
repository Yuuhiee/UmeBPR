#pragma once

#include "Image.h"

namespace Ume
{
    struct TextureSpecification
    {
        ImageFormat Format = ImageFormat::RGBA;
        TextureFilter Filter = TextureFilter::Nearest;
        TextureWrap Wrap = TextureWrap::Repeat;

        bool GenMips = false;
        bool SRGB = false;
        bool Anisotropy = false;
        // uint32_t Width = 1;
        // uint32_t Height = 1;
        static void Copy(TextureSpecification *dest, const TextureSpecification &from)
        {
            dest->Format = from.Format;
            dest->Filter = from.Filter;
            dest->Wrap = from.Wrap;
            dest->GenMips = from.GenMips;
            dest->SRGB = from.SRGB;
            dest->Anisotropy = from.Anisotropy;
        }
    };

    class Texture
    {
    public:
        virtual ~Texture() = default;

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;

        virtual void SetData(void *data, uint32_t size) = 0;

        virtual void Bind(int slot = 0) const = 0;

        virtual uint32_t GetRendererID() const = 0;

        virtual const std::string& GetFilePath() = 0;

        virtual bool operator==(const Texture &other) const = 0;

    };

    class Texture2D : public Texture
    {
    public:
        static Ref<Texture2D> Create(uint32_t width, uint32_t height, const TextureSpecification &specification = {});
        static Ref<Texture2D> Create(const std::string &path, const TextureSpecification &specification = {});
        static void BindByID(uint32_t rendererID, int slot = 0);
    };

    class TextureCube : public Texture
    {
    public:
        static Ref<TextureCube> Create(const std::string &path, const TextureSpecification &specification = {});
        static Ref<TextureCube> Create(const std::string &directory, const std::array<std::string, 6> &faces, const std::string &ext, const TextureSpecification &specification = {}); // For cubemaps
    };
}
