#pragma once

#include <glm/glm.hpp>

namespace ec
{

class MaterialTexture
{
public:

    explicit MaterialTexture(uint32_t textureIndex, uint32_t texCoords = 0) :
      m_textureIndex(textureIndex),
      m_texCoords(texCoords){};

private:

    uint32_t m_textureIndex;
    uint32_t m_texCoords{};
};

class Material
{
public:

    // TODO: Create an "error" material (e.g. pink emissive texture)

    Material() = default;

private:

    std::string m_name{};

    // PBR metallic-roughness model parameters
    glm::vec4 m_baseColor{ 1.f };
    float m_metalness{ 1.f };
    float m_roughness{ 1.f };

    MaterialTexture m_baseColorTexture;
    MaterialTexture m_metalnessRoughnessTexture;

    bool m_hasColorTexture{};
    bool m_hasMetalnessTexture{};
    bool m_hasRoughnessTexture{};

    enum class AlphaMode {
        opaque,
        mask,
        blend,
    } m_alphaMode{};
    float m_alphaCutoff{ 0.5f };  // Threshold for AlphaMode::mask

    bool m_doubleSided{};  // If true, back-face culling should be disabled

    // Additional parameters
    glm::vec3 m_emissive{};
    float m_normalScale{ 1.f };
    float m_occlusionStrength{ 1.f };

    MaterialTexture m_emissiveTexture;
    MaterialTexture m_normalTexture;
    MaterialTexture m_occlusionTexture;

    bool m_hasNormalTexture{};
    bool m_hasOcclusionTexture{};
    bool m_hasEmissiveTexture{};
};

}  // namespace ec
