#pragma once

#include "scene.hpp"
#include "entity.hpp"

namespace ec
{

// Data that can be read from a glTF file

std::vector<Scene> load_gltf_file(std::string_view filePath);

}  // namespace ec