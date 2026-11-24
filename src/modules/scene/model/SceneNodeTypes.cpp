#include "SceneNodeTypes.h"
#include "SceneNode.h"

#include <cassert>

namespace editor
{

//
// Interface
//

std::unique_ptr<CSceneNode> CSceneNodeTypes::Create(const std::string & _TypeId) const
{
  const auto TypeIterator = m_Types.find(_TypeId);
  return TypeIterator == m_Types.end() ? nullptr : TypeIterator->second.Factory();
}

const TPropertySchema & CSceneNodeTypes::GetSchema(const std::string & _TypeId) const
{
  static const TPropertySchema EmptySchema;

  const auto TypeIterator = m_Types.find(_TypeId);
  return TypeIterator == m_Types.end() ? EmptySchema : TypeIterator->second.Schema;
}

ESceneNodeIcon CSceneNodeTypes::GetIcon(const std::string & _TypeId) const
{
  const auto TypeIterator = m_Types.find(_TypeId);
  return TypeIterator == m_Types.end() ? ESceneNodeIcon::None : TypeIterator->second.Info.Icon;
}

std::vector<TSceneNodeTypeInfo> CSceneNodeTypes::GetTypes() const
{
  std::vector<TSceneNodeTypeInfo> Types;
  Types.reserve(m_Order.size());

  for (const auto & TypeId : m_Order)
    Types.push_back(m_Types.at(TypeId).Info);

  return Types;
}

//
// Implementation
//

void CSceneNodeTypes::Register(TSceneNodeTypeInfo _Info, TFactory _Factory)
{
  const auto Prototype = _Factory();
  assert(Prototype && Prototype->GetTypeId() == _Info.TypeId && "Scene node factory creates another type");

  TPropertySchema Schema{"scene.node." + _Info.TypeId, {}};
  Prototype->DescribeProperties(Schema.Properties);

  const std::string TypeId = _Info.TypeId;
  const auto [TypeIterator, WasInserted] = m_Types.emplace(TypeId, TEntry{std::move(_Info), std::move(_Factory), std::move(Schema)});
  assert(WasInserted && "Scene node type already registered");
  (void)TypeIterator;

  m_Order.push_back(TypeId);
}

} // namespace editor
