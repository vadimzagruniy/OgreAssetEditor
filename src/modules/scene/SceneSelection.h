#pragma once

#include <string>

namespace editor
{

class CSceneSelection final
{
public: // Interface

  const std::string & GetNode() const noexcept;

  void Select(std::string _Name);

  void Clear() noexcept;

private: // Attributes

  std::string m_Node;
};

} // namespace editor
