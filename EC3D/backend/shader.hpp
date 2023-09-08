#pragma once

#include <spirv_cross/spirv_cross.hpp>
#include <unordered_map>
#include <set>

namespace ec::vulkan
{

class Shader
{
public:

    Shader() = default;

    void init_from_spirv(std::string_view filePath);
    void init_from_glsl(std::string_view filePath);

    vk::ShaderModule create_shader_module(vk::Device device);

    inline const auto& get_descriptor_set_bindings() const { return m_setBindings; }
    inline const auto& get_push_constant_range() const { return m_pushConstantRange; }

    // Initializes descriptor set layout create infos and push constant ranges with reflection API
    // ShaderModule should be created before this call
    void init_resources();

    void free(vk::Device device);

private:

    void insert_binding(const spirv_cross::Compiler& comp,
                        const spirv_cross::Resource& resource,
                        vk::DescriptorType descriptorType);

private:

    vk::ShaderModule m_shaderModule{};
    std::vector<uint32_t> m_code{};

    // setBindings[i] contains the bindings of set i, to facilitate pipeline layout creation
    std::unordered_map<uint32_t, std::set<vk::DescriptorSetLayoutBinding>> m_setBindings{};
    std::optional<vk::PushConstantRange> m_pushConstantRange{};
};

}  // namespace ec::vulkan
