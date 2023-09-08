#pragma once

namespace ec
{

class Entity
{
public:

    Entity() = default;

private:

    std::string m_name{};

    std::vector<Entity> m_children{};
};

}  // namespace ec
