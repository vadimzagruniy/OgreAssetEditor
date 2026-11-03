#include "MeshDropHandler.h"
#include "SceneDrop.h"
#include "modules/scene/SceneDocument.h"
#include "modules/scene/SceneInstance.h"
#include "modules/scene/model/SceneNodeProperties.h"

namespace editor
{

namespace
{

constexpr const char * MeshAssetType = "mesh";

std::string GetParentName(const TSceneDrop & _Drop)
{
  const auto * OutlinerTarget = std::get_if<TOutlinerDropTarget>(&_Drop.Target);
  return OutlinerTarget ? OutlinerTarget->ParentName : std::string();
}

} // namespace

//
// ISceneDropHandler
//

bool CMeshDropHandler::CanDrop(const TSceneDrop & _Drop, const CSceneDocument & _Document) const
{
  if (_Drop.AssetType != MeshAssetType)
    return false;

  const std::string ParentName = GetParentName(_Drop);
  return ParentName.empty() || _Document.GetInstance().HasNode(ParentName);
}

std::string CMeshDropHandler::GetDropHint(const TSceneDrop & _Drop) const
{
  const std::string Hint       = "Add " + _Drop.AssetPath.stem().string();
  const std::string ParentName = GetParentName(_Drop);

  return ParentName.empty() ? Hint : Hint + " to " + ParentName;
}

void CMeshDropHandler::Drop(const TSceneDrop & _Drop, CSceneDocument & _Document) const
{
  TSceneNodeProperties Properties;
  Properties.MeshName = _Drop.AssetPath.generic_string();

  if (const auto * ViewportTarget = std::get_if<TViewportDropTarget>(&_Drop.Target))
    Properties.Transform.Position = ViewportTarget->Position;

  _Document.CreateNode(GetParentName(_Drop), _Drop.AssetPath.stem().string(), Properties);
}

} // namespace editor
