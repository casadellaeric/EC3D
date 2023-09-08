#include "pch.hpp"
#include "pipeline.hpp"
#include <set>

namespace ec::vulkan
{

PipelineManager::PipelineManager(const PipelineManager::Info& info) :
  m_device{ info.device }
{
}

uint32_t PipelineManager::create_basic_graphics_pipeline(const GraphicsPipelineInfo& info)
{
    // -- Shader Creation --
    // TODO: Maybe separate shaders from pipelines if many pipelines are created with same shaders
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages(info.shaders.size());
    std::vector<Shader> shaders(2);

    int fragmentShaderIdx{ -1 };
    for (size_t i = 0; i < info.shaders.size(); ++i) {
        shaders[i].init_from_spirv(info.shaders[i].filePath);
        vk::ShaderModule shaderModule{ shaders[i].create_shader_module(m_device) };
        shaderStages[i] = {
            .stage  = info.shaders[i].stage,
            .module = shaderModule,
            .pName  = "main",
        };
        if (info.shaders[i].stage == vk::ShaderStageFlagBits::eFragment) {
            fragmentShaderIdx = static_cast<int>(i);
        }
    }

    // -- Vertex Input --
    std::vector<vk::VertexInputBindingDescription> vertexInputBindingDescriptions(
        info.bindingStrides.size());
    std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions(
        info.attributeDescriptions.size());

    for (size_t i = 0; i < info.bindingStrides.size(); ++i) {
        vertexInputBindingDescriptions[i] = {
            .binding   = static_cast<uint32_t>(i),
            .stride    = info.bindingStrides[i],
            .inputRate = vk::VertexInputRate::eVertex,
        };
    }

    for (size_t i = 0; i < info.attributeDescriptions.size(); ++i) {
        vertexInputAttributeDescriptions[i] = {
            .location = static_cast<uint32_t>(i),
            .binding  = info.attributeDescriptions[i].binding,
            .format   = info.attributeDescriptions[i].format,
            .offset   = info.attributeDescriptions[i].offset,
        };
    }

    vk::PipelineVertexInputStateCreateInfo vertexInputState{
        .vertexBindingDescriptionCount
        = static_cast<uint32_t>(vertexInputBindingDescriptions.size()),
        .pVertexBindingDescriptions = vertexInputBindingDescriptions.data(),
        .vertexAttributeDescriptionCount
        = static_cast<uint32_t>(vertexInputAttributeDescriptions.size()),
        .pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data(),
    };

    // -- Primitive Assembly --
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState{
        .topology               = info.primitiveTopology,
        .primitiveRestartEnable = info.primitiveRestartEnable,
    };

    // -- Tessellation -- (Not active for now)
    // vk::PipelineTessellationStateCreateInfo tessellationState{};

    // -- Viewport --
    // TODO: Make viewport dynamic, on window resize
    std::array<vk::Viewport, 1> viewports{};
    viewports[0] = {
        .width    = static_cast<float>(info.viewportExtent.width),
        .height   = static_cast<float>(info.viewportExtent.height),
        .minDepth = 0.f,
        .maxDepth = 1.f,
    };

    std::array<vk::Rect2D, 1> scissors{};
    scissors[0] = {
        .extent = info.viewportExtent,
    };

    vk::PipelineViewportStateCreateInfo viewportState{
        .viewportCount = static_cast<uint32_t>(viewports.size()),
        .pViewports    = viewports.data(),
        .scissorCount  = static_cast<uint32_t>(scissors.size()),
        .pScissors     = scissors.data(),
    };

    // -- Rasterization --
    vk::PipelineRasterizationStateCreateInfo rasterizationState{
        .depthClampEnable        = info.depthClampEnable,
        .rasterizerDiscardEnable = fragmentShaderIdx == -1,  // Discard if no fragment shader
        .polygonMode             = vk::PolygonMode::eFill,
        .cullMode                = info.cullMode,
        .frontFace               = vk::FrontFace::eCounterClockwise,
        .depthBiasEnable         = VK_FALSE,
        .lineWidth               = 1.f,
    };

    // -- Multisampling --
    vk::PipelineMultisampleStateCreateInfo multisampleState{
        .rasterizationSamples = info.rasterizationSamples,
        .sampleShadingEnable  = VK_FALSE,
    };

    // -- Depth Stencil --
    vk::PipelineDepthStencilStateCreateInfo depthStencilState{
        .depthTestEnable       = info.depthTestEnable,
        .depthWriteEnable      = VK_TRUE,
        .depthCompareOp        = vk::CompareOp::eLess,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable     = info.stencilTestEnable,
        .front                 = info.stencilStateFront,
        .back                  = info.stencilStateBack,
    };

    // -- Blending --
    // Basic blend formula
    vk::PipelineColorBlendAttachmentState baseBlendAttachmentState{
        .blendEnable         = VK_TRUE,
        .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
        .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
        .colorBlendOp        = vk::BlendOp::eAdd,
        .srcAlphaBlendFactor = vk::BlendFactor::eOne,
        .dstAlphaBlendFactor = vk::BlendFactor::eZero,
        .alphaBlendOp        = vk::BlendOp::eAdd,
        .colorWriteMask      = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
                          | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
    };
    // Set blending enabled per color attachment
    std::vector<vk::PipelineColorBlendAttachmentState> blendAttachments(
        info.blendEnableInAttachments.size(),
        baseBlendAttachmentState);
    for (size_t i = 0; i < blendAttachments.size(); ++i) {
        blendAttachments[i].blendEnable = info.blendEnableInAttachments[i];
    }
    vk::PipelineColorBlendStateCreateInfo colorBlendState{
        .logicOpEnable   = VK_FALSE,
        .logicOp         = vk::LogicOp::eNoOp,
        .attachmentCount = static_cast<uint32_t>(blendAttachments.size()),
        .pAttachments    = blendAttachments.data(),
    };

    // -- Dynamic State -- (TODO)
    // vk::PipelineDynamicStateCreateInfo dynamicState{};

    // -- Layout --
    uint32_t maxSetIdx{};  // Store biggest set index to later create big enough array
    std::unordered_map<uint32_t, std::set<vk::DescriptorSetLayoutBinding>>
        pipelineSetUniqueBindings{};
    // Add bindings to a unique set, to eliminate duplicates
    for (size_t i = 0; i < shaders.size(); ++i) {
        shaders[i].init_resources();
        for (auto& set : shaders[i].get_descriptor_set_bindings()) {
            // Find highest set idx
            if (set.first > maxSetIdx) {
                maxSetIdx = set.first;
            }
            pipelineSetUniqueBindings[set.first].insert(set.second.begin(), set.second.end());
        }
    }

    // Convert set to vector to pass to createInfo
    std::unordered_map<uint32_t, std::vector<vk::DescriptorSetLayoutBinding>> pipelineSetBindings{};
    for (auto& set : pipelineSetUniqueBindings) {
        pipelineSetBindings[set.first]
            = std::vector<vk::DescriptorSetLayoutBinding>(set.second.begin(), set.second.end());
    }

    // For all the common bindings, find, for each shader, if they are used and update flags
    for (size_t i = 0; i < shaders.size(); ++i) {
        for (auto& shaderSet : shaders[i].get_descriptor_set_bindings()) {
            auto& commonBindings{ pipelineSetBindings[shaderSet.first] };
            for (auto& shaderBinding : shaderSet.second) {
                for (size_t j = 0; j < commonBindings.size(); ++j) {
                    if (commonBindings[j].binding == shaderBinding.binding) {
                        commonBindings[j].stageFlags |= info.shaders[i].stage;
                    }
                }
            }
        }
    }

    // Create final descriptor set layouts and check for support
    std::vector<vk::DescriptorSetLayout> setLayouts{};
    if (pipelineSetBindings.size() > 0) {
        setLayouts.resize(maxSetIdx + 1, VK_NULL_HANDLE);
    }
    for (auto& set : pipelineSetBindings) {
        vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{
            .bindingCount = static_cast<uint32_t>(set.second.size()),
            .pBindings    = set.second.data(),
        };
        if (!m_device.getDescriptorSetLayoutSupport(descriptorSetLayoutCreateInfo).supported) {
            throw std::runtime_error("Descriptor set not supported!");
        }
        setLayouts[set.first] = m_device.createDescriptorSetLayout(descriptorSetLayoutCreateInfo);
    }

    std::set<vk::PushConstantRange> uniquePushConstantRanges{};
    for (auto& shader : shaders) {
        if (shader.get_push_constant_range().has_value()) {
            uniquePushConstantRanges.insert(shader.get_push_constant_range().value());
        }
    }
    std::vector<vk::PushConstantRange> pushConstantRanges{ std::vector<vk::PushConstantRange>(
        uniquePushConstantRanges.begin(),
        uniquePushConstantRanges.end()) };

    for (size_t i = 0; i < shaders.size(); ++i) {
        auto& optRange{ shaders[i].get_push_constant_range() };
        if (!optRange.has_value()) {
            continue;
        }
        auto& shaderRange{ optRange.value() };
        for (auto& pushConstantRange : pushConstantRanges) {
            if (pushConstantRange.size == shaderRange.size
                && pushConstantRange.offset == shaderRange.offset) {
                pushConstantRange.stageFlags |= info.shaders[i].stage;
            }
        }
    }

    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{
        .setLayoutCount         = static_cast<uint32_t>(setLayouts.size()),
        .pSetLayouts            = setLayouts.data(),
        .pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size()),
        .pPushConstantRanges    = pushConstantRanges.data(),
    };
    m_pipelineLayout = m_device.createPipelineLayout(pipelineLayoutCreateInfo);

    // -- Pipeline --
    vk::GraphicsPipelineCreateInfo pipelineCreateInfo{
        .stageCount          = static_cast<uint32_t>(shaderStages.size()),
        .pStages             = shaderStages.data(),
        .pVertexInputState   = &vertexInputState,
        .pInputAssemblyState = &inputAssemblyState,
        .pViewportState      = &viewportState,
        .pRasterizationState = &rasterizationState,
        .pMultisampleState   = &multisampleState,
        .pDepthStencilState  = &depthStencilState,
        .pColorBlendState    = &colorBlendState,
        .layout              = m_pipelineLayout,
        .renderPass          = info.renderPass,
        .subpass             = info.subpassIdx,
    };

    m_pipeline = m_device.createGraphicsPipeline(VK_NULL_HANDLE, pipelineCreateInfo).value;
    for (auto& shader : shaders) {
        shader.free(m_device);
    }
    return 0;
}

void PipelineManager::free()
{
    if (m_pipeline) {
        m_device.destroyPipeline(m_pipeline);
        m_pipeline = nullptr;
    }
    if (m_pipelineLayout) {
        m_device.destroyPipelineLayout(m_pipelineLayout);
        m_pipelineLayout = nullptr;
    }
}

}  // namespace ec::vulkan
