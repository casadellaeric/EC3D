#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include "backend/shader.hpp"

namespace klast::vulkan
{

struct ShaderInfo {
    std::string_view filePath{};
    vk::ShaderStageFlagBits stage{};
};

struct VertexAttributeDescription {
    vk::Format format{};
    uint32_t binding{};
    uint32_t offset{};
};

struct GraphicsPipelineInfo {
    std::vector<ShaderInfo> shaders{};
    std::vector<uint32_t> bindingStrides{};
    std::vector<VertexAttributeDescription> attributeDescriptions{};
    vk::PrimitiveTopology primitiveTopology{ vk::PrimitiveTopology::eTriangleList };
    vk::Bool32 primitiveRestartEnable{ VK_FALSE };
    vk::Bool32 depthClampEnable{ VK_FALSE };
    vk::Extent2D viewportExtent{};
    vk::CullModeFlags cullMode{ vk::CullModeFlagBits::eBack };
    vk::SampleCountFlagBits rasterizationSamples{ vk::SampleCountFlagBits::e1 };
    vk::Bool32 depthTestEnable{};
    vk::Bool32 stencilTestEnable{};
    vk::StencilOpState stencilStateFront{};
    vk::StencilOpState stencilStateBack{};
    std::vector<vk::Bool32> blendEnableInAttachments{};
    vk::RenderPass renderPass{};
    uint32_t subpassIdx{};
};

class PipelineManager
{
public:

    struct Info {
        vk::Device device;
    };

    PipelineManager() = default;
    PipelineManager(const PipelineManager::Info& info);

    uint32_t create_basic_graphics_pipeline(const GraphicsPipelineInfo& info);

    vk::Pipeline get_pipeline() const { return m_pipeline; }

    void free();

private:

    vk::Device m_device{};

    vk::Pipeline m_pipeline{};
    vk::PipelineLayout m_pipelineLayout{};
};

}  // namespace klast::vulkan
