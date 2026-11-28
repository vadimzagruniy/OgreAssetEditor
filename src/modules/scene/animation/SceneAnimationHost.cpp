#include "SceneAnimationHost.h"
#include "editor/assets/AssetReference.h"
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

CSceneAnimationHost::CSceneAnimationHost(std::shared_ptr<CSceneDocument> _Document, std::shared_ptr<CSceneSelection> _Selection, CSceneSync & _Sync):
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

void CSceneAnimationHost::SelectNode(const std::string & _Name)
{
  m_Selection->Select(_Name);
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

std::vector<TAssetReference> CSceneAnimationHost::GetAnimationRefs() const
{
  std::vector<TAssetReference> AnimationRefs;

  for (const auto & Animation : m_Document->GetAnimations())
    AnimationRefs.push_back(Animation.AssetRef);

  return AnimationRefs;
}

bool CSceneAnimationHost::HasAnimation(const TAssetId & _AnimationId) const
{
  return m_Document->HasAnimation(_AnimationId);
}

void CSceneAnimationHost::AddAnimation(const TAssetReference & _AnimationRef)
{
  m_Document->AddAnimation(_AnimationRef);
}

void CSceneAnimationHost::RemoveAnimation(const TAssetReference & _AnimationRef)
{
  m_Document->RemoveAnimation(_AnimationRef);
}

std::string CSceneAnimationHost::ResolveSlot(const TAssetId & _AnimationId, const std::string & _Slot) const
{
  return m_Document->ResolveSlot(_AnimationId, _Slot);
}

std::string CSceneAnimationHost::FindSlot(const TAssetId & _AnimationId, const std::string & _NodeName) const
{
  return m_Document->FindSlot(_AnimationId, _NodeName);
}

void CSceneAnimationHost::BindSlot(const TAssetId & _AnimationId, const std::string & _Slot, const std::string & _NodeName)
{
  m_Document->BindSlot(_AnimationId, _Slot, _NodeName);
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
