#include "AnimationAssetType.h"
#include "AnimationDocument.h"
#include "AnimationEditor.h"
#include "core/EditorService.h"
#include "editor/assets/AssetRecord.h"
#include "editor/assets/AssetTypeInfo.h"
#include "editor/workspaces/EditorWorkspace.h"
#include "editor/workspaces/WorkspaceManager.h"
#include "modules/scene/animation/ISceneAnimationHost.h"
#include "views/AnimationTimelineWindow.h"
#include "core/PathUtf8.h"

#include <fstream>
#include <iterator>
#include <utility>

namespace editor
{

//
// Constructor
//

CAnimationAssetType::CAnimationAssetType(std::shared_ptr<CWorkspaceManager> _WorkspaceManager, std::shared_ptr<CAnimationTimelineWindow> _TimelineWindow, std::shared_ptr<core::CActionQueue> _ActionQueue):
  m_WorkspaceManager(std::move(_WorkspaceManager)),
  m_TimelineWindow(std::move(_TimelineWindow)),
  m_ActionQueue(std::move(_ActionQueue))
{
  // Empty
}

//
// IAssetType
//

const TAssetTypeInfo & CAnimationAssetType::GetInfo() const noexcept
{
  static const TAssetTypeInfo Info{"animation", "Animation", {".oanim"}, true};
  return Info;
}

std::unique_ptr<CEditorWorkspace> CAnimationAssetType::OpenAsset(const TAssetRecord & _Asset, const std::string & /*_ResourceGroup*/, std::string & _Error) const
{
  const auto HostIndex = m_WorkspaceManager->FindHost<ISceneAnimationHost>();
  if (!HostIndex)
  {
    _Error = "Open a scene to edit " + _Asset.RelativePath.filename().string() + ".";
    return {};
  }

  m_WorkspaceManager->SetActive(*HostIndex);

  if (!m_WorkspaceManager->FindAttachedEditor(*HostIndex, _Asset.AbsolutePath) && !AttachNewEditor(*HostIndex, _Asset, _Error))
    return {};

  m_TimelineWindow->ShowAnimation(_Asset.AbsolutePath);

  return {};
}

//
// Implementation
//

bool CAnimationAssetType::AttachNewEditor(std::size_t _HostIndex, const TAssetRecord & _Asset, std::string & _Error) const
{
  std::ifstream File(_Asset.AbsolutePath, std::ios::binary);
  if (!File)
  {
    _Error = "Could not open animation file: " + core::PathToUtf8(_Asset.AbsolutePath);
    return false;
  }

  const std::string Text((std::istreambuf_iterator<char>(File)), {});

  auto Document = std::make_shared<CAnimationDocument>(_Asset);
  if (!Document->Load(Text, _Error))
  {
    _Error = _Asset.RelativePath.generic_string() + ": " + _Error;
    return false;
  }

  auto Host = m_WorkspaceManager->GetWorkspaces()[_HostIndex]->GetServices().GetService<ISceneAnimationHost>();

  return m_WorkspaceManager->AttachEditor(_HostIndex, std::make_shared<CAnimationEditor>(std::move(Document), std::move(Host), m_ActionQueue));
}

} // namespace editor
