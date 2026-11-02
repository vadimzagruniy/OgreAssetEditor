#pragma once

#include <functional>
#include <vector>

namespace editor::core
{

class CActionQueue
{
public: // Interface

  void Post(std::function<void()> _Action);

  void Apply();

private: // Attributes

  std::vector<std::function<void()>> m_Actions;
};

} // namespace editor::core
