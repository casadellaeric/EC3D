#pragma once

#include "entity.hpp"

#include <string>

namespace ec
{

class Scene
{
public:

    Scene() = default;

private:

    std::string m_name{};

    std::vector<Entity> m_entities{};
};

}  // namespace ec
