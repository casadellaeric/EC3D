#include "pch.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#pragma warning(push, 0)
#include <tiny_gltf.h>
#pragma warning(pop)

namespace ec
{

using namespace tinygltf;

std::vector<Scene> load_gltf_file(std::string_view filePath)
{
    std::filesystem::path path(filePath);
    auto extension{ path.extension().string() };

    Model root{};
    TinyGLTF loader{};
    std::string error{}, warning{};

    bool ret{};
    if (extension == ".gltf") {
        ret = loader.LoadASCIIFromFile(&root, &error, &warning, path.string());
    } else if (extension == ".glb") {
        ret = loader.LoadBinaryFromFile(&root, &error, &warning, path.string());
    } else {
        throw std::runtime_error(
            std::format("Failed to open glTF file {}. Extension not supported!\n", filePath));
    }
    if (!warning.empty()) {
        EC_LOG_WARN("Warning loading glTF file: {}\n", warning.c_str());
    }
    if (!ret) {
        throw std::runtime_error(
            std::format("Failed to load glTF file. Error: {}\n", error.c_str()));
    }

    std::vector<Scene> scenes(root.scenes.size());

}

}  // namespace ec