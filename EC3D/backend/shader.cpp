#include "pch.hpp"
#include "shader.hpp"
#include "backend/utils.hpp"

#include <string_view>
#include <algorithm>
#include <ranges>
#include <numeric>

namespace ec::vulkan
{

void Shader::init_from_spirv(std::string_view filePath)
{
    m_code = read_file(filePath);
}

vk::ShaderModule Shader::create_shader_module(vk::Device device)
{
    vk::ShaderModuleCreateInfo shaderModuleCreateInfo{
        .codeSize = static_cast<uint32_t>(m_code.size() * sizeof(uint32_t)),
        .pCode    = reinterpret_cast<const uint32_t*>(m_code.data()),
    };

    m_shaderModule = device.createShaderModule(shaderModuleCreateInfo);
    return m_shaderModule;
}

void Shader::free(vk::Device device)
{
    if (m_shaderModule) {
        device.destroyShaderModule(m_shaderModule);
        m_shaderModule = nullptr;
    }
    m_code.clear();
}

void Shader::insert_binding(const spirv_cross::Compiler& comp,
                            const spirv_cross::Resource& resource,
                            vk::DescriptorType descriptorType)
{
    uint32_t set{ comp.get_decoration(resource.id, spv::DecorationDescriptorSet) };
    uint32_t binding{ comp.get_decoration(resource.id, spv::DecorationBinding) };

    m_setBindings[set].emplace(vk::DescriptorSetLayoutBinding{
        .binding         = binding,
        .descriptorType  = descriptorType,
        .descriptorCount = 1,
        .stageFlags      = {}  // To be filled by pipeline
    });
}

void Shader::init_resources()
{
    // TODO: Remove std::move if the shader is to be used many times
    spirv_cross::Compiler comp(m_code);
    auto resources = comp.get_shader_resources();

    for (const auto& resource : resources.sampled_images) {
        insert_binding(comp, resource, vk::DescriptorType::eCombinedImageSampler);
    }
    for (const auto& resource : resources.separate_images) {
        const auto& type{ comp.get_type(resource.base_type_id) };
        switch (type.image.dim) {
            case spv::Dim::DimBuffer:
                insert_binding(comp, resource, vk::DescriptorType::eUniformTexelBuffer);
                break;
            default: insert_binding(comp, resource, vk::DescriptorType::eSampledImage);
        }
    }
    for (const auto& resource : resources.storage_images) {
        const auto& type{ comp.get_type(resource.base_type_id) };
        switch (type.image.dim) {
            case spv::Dim::DimBuffer:
                insert_binding(comp, resource, vk::DescriptorType::eStorageTexelBuffer);
                break;
            default: insert_binding(comp, resource, vk::DescriptorType::eStorageImage);
        }
    }
    for (const auto& resource : resources.separate_samplers) {
        insert_binding(comp, resource, vk::DescriptorType::eSampler);
    }
    // TODO: Dynamic uniform buffers
    for (const auto& resource : resources.uniform_buffers) {
        insert_binding(comp, resource, vk::DescriptorType::eUniformBuffer);
    }
    for (const auto& resource : resources.subpass_inputs) {
        insert_binding(comp, resource, vk::DescriptorType::eInputAttachment);
    }
    // TODO: Dynamic storage buffers
    for (const auto& resource : resources.storage_buffers) {
        insert_binding(comp, resource, vk::DescriptorType::eStorageBuffer);
    }

    for (const auto& resource : resources.push_constant_buffers) {
        std::vector<spirv_cross::BufferRange> ranges{ comp.get_active_buffer_ranges(resource.id) };
        size_t minOffset{ std::numeric_limits<uint32_t>::max() };
        size_t totalSize{};
        for (auto& range : ranges) {
            if (range.offset < minOffset) {
                minOffset = range.offset;
            }
            totalSize += range.range;
        }
        m_pushConstantRange.emplace(vk::PushConstantRange{
            .stageFlags = {},  // To be filled by pipeline
            .offset     = static_cast<uint32_t>(minOffset),
            .size       = static_cast<uint32_t>(totalSize),
        });
    }
}

}  // namespace ec::vulkan
