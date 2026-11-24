#pragma once

#include "SceneNodeTypeInfo.h"
#include "editor/inspector/PropertySchema.h"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace editor
{

class CSceneNode;

class CSceneNodeTypes final
{
public: // Types

  using TFactory = std::function<std::unique_ptr<CSceneNode>()>;

public: // Interface

  template <class TNode>
  void Register(std::string _Label, std::string _Category);

  std::unique_ptr<CSceneNode> Create(const std::string & _TypeId) const;

  const TPropertySchema & GetSchema(const std::string & _TypeId) const;

  std::vector<TSceneNodeTypeInfo> GetTypes() const;

private: // Types

  struct TEntry
  {
    TSceneNodeTypeInfo Info;
    TFactory           Factory;
    TPropertySchema    Schema;
  };

private: // Implementation

  void Register(TSceneNodeTypeInfo _Info, TFactory _Factory);

private: // Attributes

  std::unordered_map<std::string, TEntry> m_Types;
  std::vector<std::string>                m_Order;
};

//
// Template implementation
//

template <class TNode>
void CSceneNodeTypes::Register(std::string _Label, std::string _Category)
{
  Register(TSceneNodeTypeInfo{std::string(TNode::TypeId), std::move(_Label), std::move(_Category)}, []
  {
    return std::make_unique<TNode>();
  });
}

} // namespace editor
