#pragma once

#include "SceneNodeTemplate.h"

#include <optional>
#include <string>
#include <vector>

namespace editor
{

class CSceneNode;

class CSceneNodeTemplates final
{
public: // Interface

  void Register(std::string _Identifier, std::string _Label, std::string _Category, const CSceneNode & _Prototype);

  std::optional<TSceneNodeTemplate> Find(const std::string & _Identifier) const;

  const std::vector<TSceneNodeTemplate> & GetTemplates() const noexcept;

private: // Attributes

  std::vector<TSceneNodeTemplate> m_Templates;
};

} // namespace editor
