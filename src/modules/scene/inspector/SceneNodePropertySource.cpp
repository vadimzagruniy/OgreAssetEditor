#include "SceneNodePropertySource.h"
#include "editor/inspector/PropertySchema.h"
#include "modules/scene/SceneDocument.h"
#include "modules/scene/SceneSelection.h"
#include "modules/scene/model/SceneGraph.h"
#include "modules/scene/model/SceneNode.h"
#include "modules/scene/model/SceneNodeTypes.h"

#include <utility>

namespace editor
{

namespace
{

const std::string TransformPrefix = "Transform.";

} // namespace

//
// Constructor
//

CSceneNodePropertySource::CSceneNodePropertySource(std::shared_ptr<CSceneDocument> _Document, std::shared_ptr<const CSceneSelection> _Selection):
  m_Document(std::move(_Document)),
  m_Selection(std::move(_Selection))
{
  // Empty
}

//
// IPropertySource
//

std::string CSceneNodePropertySource::GetTitle() const
{
  return m_Selection->GetNode();
}

const TPropertySchema & CSceneNodePropertySource::GetSchema() const
{
  static const TPropertySchema EmptySchema;

  if (!HasSelectedNode())
    return EmptySchema;

  const auto & Graph = m_Document->GetGraph();
  return Graph.GetNodeTypes().GetSchema(std::string(Graph.GetNode(m_Selection->GetNode()).GetTypeId()));
}

TPropertyValue CSceneNodePropertySource::GetValue(const std::string & _Identifier) const
{
  if (!HasSelectedNode())
    return {};

  return m_Document->GetGraph().GetNode(m_Selection->GetNode()).GetValue(_Identifier);
}

void CSceneNodePropertySource::Preview(const std::string & _Identifier, const TPropertyValue & _Value)
{
  if (!HasSelectedNode())
    return;

  const bool IsTransform = _Identifier.starts_with(TransformPrefix);

  m_Document->PreviewValue(m_Selection->GetNode(), _Identifier, _Value, IsTransform ? "Edit Transform" : "Edit Properties");
}

void CSceneNodePropertySource::Commit()
{
  m_Document->CommitPreview();
}

//
// Implementation
//

bool CSceneNodePropertySource::HasSelectedNode() const
{
  const auto & Selection = m_Selection->GetNode();
  return !Selection.empty() && m_Document->GetGraph().HasNode(Selection);
}

} // namespace editor
