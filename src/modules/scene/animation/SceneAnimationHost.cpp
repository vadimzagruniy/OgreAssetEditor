#include "SceneAnimationHost.h"
#include "modules/scene/SceneDocument.h"
#include "modules/scene/SceneSelection.h"
#include "modules/scene/model/SceneGraph.h"
#include "modules/scene/model/SceneNode.h"
#include "modules/scene/model/SceneNodeSnapshot.h"
#include "modules/scene/model/SceneNodeTypes.h"
#include "modules/scene/render/SceneSync.h"

#include <utility>

namespace editor
{

//
// Constructor
//

CSceneAnimationHost::CSceneAnimationHost(std::shared_ptr<CSceneDocument> _Document, std::shared_ptr<const CSceneSelection> _Selection, CSceneSync & _Sync):
  m_Document(std::move(_Document)),
  m_Selection(std::move(_Selection)),
  m_Sync(_Sync)
{
  // Empty
}

//
// ISceneAnimationHost
//

const CSceneGraph & CSceneAnimationHost::GetGraph() const
{
  return m_Document->GetGraph();
}

std::string CSceneAnimationHost::GetSelectedNode() const
{
  return m_Selection->GetNode();
}

void CSceneAnimationHost::PreviewValue(const std::string & _Name, const std::string & _Identifier, const TPropertyValue & _Value)
{
  m_PreviewValues[_Name][_Identifier] = _Value;
}

void CSceneAnimationHost::RestoreNode(const std::string & _Name)
{
  m_PreviewValues.erase(_Name);
  m_Sync.RestoreNode(m_Document->GetGraph(), _Name);
}

std::vector<std::string> CSceneAnimationHost::GetAnimations() const
{
  std::vector<std::string> Paths;

  for (const auto & Animation : m_Document->GetAnimations())
    Paths.push_back(Animation.Path);

  return Paths;
}

bool CSceneAnimationHost::HasAnimation(const std::string & _Path) const
{
  return m_Document->HasAnimation(_Path);
}

void CSceneAnimationHost::AddAnimation(const std::string & _Path)
{
  m_Document->AddAnimation(_Path);
}

void CSceneAnimationHost::RemoveAnimation(const std::string & _Path)
{
  m_Document->RemoveAnimation(_Path);
}

std::string CSceneAnimationHost::ResolveSlot(const std::string & _Path, const std::string & _Slot) const
{
  return m_Document->ResolveSlot(_Path, _Slot);
}

std::string CSceneAnimationHost::FindSlot(const std::string & _Path, const std::string & _NodeName) const
{
  return m_Document->FindSlot(_Path, _NodeName);
}

void CSceneAnimationHost::BindSlot(const std::string & _Path, const std::string & _Slot, const std::string & _NodeName)
{
  m_Document->BindSlot(_Path, _Slot, _NodeName);
}

//
// Interface
//

void CSceneAnimationHost::ApplyPreview()
{
  const auto & Graph = m_Document->GetGraph();

  for (const auto & [NodeName, Values] : m_PreviewValues)
  {
    if (!Graph.HasNode(NodeName))
      continue;

    const auto & Source  = Graph.GetNode(NodeName);
    auto         Preview = Graph.GetNodeTypes().Create(std::string(Source.GetTypeId()));

    std::string Error;
    if (!Preview || !Preview->ApplySnapshot(Source.CaptureSnapshot(), Error))
      continue;

    for (const auto & [Identifier, Value] : Values)
      Preview->SetValue(Identifier, Value);

    m_Sync.ApplyPreview(NodeName, *Preview);
  }
}

} // namespace editor
