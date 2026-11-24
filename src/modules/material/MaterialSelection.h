#pragma once

#include "MaterialNode.h"

namespace editor
{

class CMaterialSelection final
{
public: // Interface

  const TMaterialNode & GetNode() const noexcept;

  void Select(const TMaterialNode & _Node) noexcept;

private: // Attributes

  TMaterialNode m_Node;
};

} // namespace editor
