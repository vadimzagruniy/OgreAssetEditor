#pragma once

#include "editor/assets/AssetType.h"

#include <memory>

namespace editor
{

class CAnimationTimelineWindow;
class CWorkspaceManager;

class CAnimationAssetType final:
  public IAssetType
{
public: // Constructor

  CAnimationAssetType(std::shared_ptr<CWorkspaceManager> _WorkspaceManager, std::shared_ptr<CAnimationTimelineWindow> _TimelineWindow);

public: // IAssetType

  const TAssetTypeInfo & GetInfo() const noexcept override;

  std::unique_ptr<CEditorWorkspace> OpenAsset(const TAssetRecord & _Asset, const std::string & _ResourceGroup, std::string & _Error) const override;

private: // Implementation

  bool AttachNewEditor(std::size_t _HostIndex, const TAssetRecord & _Asset, std::string & _Error) const;

private: // Attributes

  std::shared_ptr<CWorkspaceManager>        m_WorkspaceManager;
  std::shared_ptr<CAnimationTimelineWindow> m_TimelineWindow;
};

} // namespace editor
