#include "umepch.h"
#include "KullaConty.h"

#include "stb_image_write.h"
#include "stb_image.h"
#include "glm/glm.hpp"
#include <fstream>

#include "Utils.h"

namespace Ume
{

    static const float PI = 3.14159265359;

    static float DistributionGGX(glm::vec3 N, glm::vec3 H, float roughness)
    {
        float a = roughness * roughness;
        float a2 = a * a;
        float NdotH = std::max(dot(N, H), 0.0f);
        float NdotH2 = NdotH * NdotH;

        float nom = a2;
        float denom = (NdotH2 * (a2 - 1.0) + 1.0);
        denom = PI * denom * denom;

        return nom / std::max(denom, 0.0001f);
    }

    static float GeometrySchlickGGX(float NdotV, float roughness)
    {
        float a = roughness;
        float k = (a * a) / 2.0f;

        float nom = NdotV;
        float denom = NdotV * (1.0f - k) + k;

        return nom / denom;
    }

    static float GeometrySmith(float roughness, float NoV, float NoL)
    {
        float ggx2 = GeometrySchlickGGX(NoV, roughness);
        float ggx1 = GeometrySchlickGGX(NoL, roughness);

        return ggx1 * ggx2;
    }

    static glm::vec2 Hammersley(uint32_t i, uint32_t N)
    {
        uint32_t bits = (i << 16u) | (i >> 16u);
        bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
        bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
        bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
        bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
        float rdi = float(bits) * 2.3283064365386963e-10;
        return { float(i) / float(N), rdi };
    }

    static glm::vec3 ImportanceSampleGGX(glm::vec2 Xi, glm::vec3 N, float roughness)
    {
        float a = roughness * roughness;

        float theta = atan(a * sqrt(Xi.x) / sqrt(1.0f - Xi.x));
        float phi = 2.0 * PI * Xi.y;

        float sinTheta = sin(theta);
        float consTheta = cos(theta);
        glm::vec3 H = glm::vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, consTheta);

        glm::vec3 up = abs(N.z) < 0.999 ? glm::vec3(0.0, 0.0, 1.0) : glm::vec3(1.0, 0.0, 0.0);
        glm::vec3 tangent = glm::normalize(glm::cross(up, N));
        glm::vec3 bitangent = glm::cross(N, tangent);

        glm::vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
        return glm::normalize(sampleVec);
    }

    static glm::vec3 IntegrateBRDF(glm::vec3 V, float roughness)
    {
        const int sample_count = 1024;
        glm::vec3 Emu(0.0f);
        glm::vec3 N = glm::vec3(0.0, 0.0, 1.0);
        for (int i = 0; i < sample_count; i++)
        {
            glm::vec2 Xi = Hammersley(i, sample_count);
            glm::vec3 H = ImportanceSampleGGX(Xi, N, roughness);
            glm::vec3 L = glm::normalize(H * 2.0f * dot(V, H) - V);

            float NoL = std::max(L.z, 0.0f);
            float NoH = std::max(H.z, 0.0f);
            float VoH = std::max(dot(V, H), 0.0f);
            float NoV = std::max(dot(N, V), 0.0f);

            float G = GeometrySmith(roughness, NoV, NoL);
            float weight = VoH * G / (NoV * NoH);
            Emu += glm::vec3(1.0) * weight;
        }
        return Emu * (1.0f / sample_count);
    }

    bool LUT::FileExist(const std::string& filepath, int width, int height, int channels)
    {
        int w, h, c;
        unsigned char* data = stbi_load(filepath.c_str(), &w, &h, &c, 0);
        bool exist = data && width == w && height == h && channels == c;
        if (exist)
            return true;
        return false;
    }

    void LUT::PrecomputeEmu(const std::string& filepath, int resolution)
    {
        if (FileExist(filepath, resolution, resolution, 3)) return;

        uint8_t* data = new uint8_t[resolution * resolution * 3];

        float step = 1.0 / resolution;
        for (int i = 0; i < resolution; i++) {
            for (int j = 0; j < resolution; j++) {
                float roughness = step * (static_cast<float>(i) + 0.5f);
                float NdotV = step * (static_cast<float>(j) + 0.5f);
                glm::vec3 V = glm::vec3(std::sqrt(1.f - NdotV * NdotV), 0.f, NdotV);

                glm::vec3 irr = IntegrateBRDF(V, roughness);

                data[(i * resolution + j) * 3 + 0] = uint8_t(irr.x * 255.0);
                data[(i * resolution + j) * 3 + 1] = uint8_t(irr.y * 255.0);
                data[(i * resolution + j) * 3 + 2] = uint8_t(irr.z * 255.0);
            }
        }
        stbi_flip_vertically_on_write(true);
        stbi_write_png(filepath.c_str(), resolution, resolution, 3, data, resolution * 3);
    }

    void LUT::PrecomputeEavg(const std::string& filepath, const std::string& emuPath, int resolution)
    {
        if (FileExist(filepath, resolution, resolution, 3)) return;

        PrecomputeEmu(emuPath);

        int channel = 3;
        unsigned char* Edata = stbi_load(emuPath.c_str(), &resolution, &resolution, &channel, 0);

        UME_CORE_ASSERT(Edata, "Error");

        uint8_t* data = new uint8_t[resolution * resolution * 3];

        auto GetEmu = [&](int x, int y) {
            return glm::vec3(
                Edata[3 * (resolution * x + y) + 0],
                Edata[3 * (resolution * x + y) + 1],
                Edata[3 * (resolution * x + y) + 2]);
        };

        auto SetRGB = [&](int x, int y, glm::vec3 value) {
            data[3 * (resolution * x + y) + 0] = uint8_t(value.x);
            data[3 * (resolution * x + y) + 1] = uint8_t(value.y);
            data[3 * (resolution * x + y) + 2] = uint8_t(value.z);
        };

        float step = 1.0 / resolution;
        glm::vec3 Eavg = glm::vec3(0.0);
        for (int i = 0; i < resolution; i++)
        {
            float roughness = step * (static_cast<float>(i) + 0.5f);
            for (int j = 0; j < resolution; j++)
            {
                float NdotV = step * (static_cast<float>(j) + 0.5f);
                glm::vec3 V = glm::vec3(std::sqrt(1.f - NdotV * NdotV), 0.f, NdotV);

                glm::vec3 Ei = GetEmu((resolution - 1 - i), j);
                Eavg += (NdotV * Ei * 2.0f) * step;
                SetRGB(i, j, glm::vec3(0.0));
            }

            for (int k = 0; k < resolution; k++)
            {
                SetRGB(i, k, Eavg);
            }

            Eavg = glm::vec3(0.0);
        }
        stbi_flip_vertically_on_write(true);
        stbi_write_png(filepath.c_str(), resolution, resolution, channel, data, 0);
    }
}