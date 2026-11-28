#pragma once

#include "editor/assets/MissingAssetReference.h"
#include "ui/DockedWindow.h"
#include "ui/elements/Button.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace editor
{

namespace core
{
enum class ELogLevel;
class CActionQueue;
class CLog;
} // namespace core

namespace ui
{
class CWindowManager;
struct TToolBoxDesc;
} // namespace ui

class CAssetDatabase;
class CAssetDependencyIndex;
class CAssetManager;
class CWorkspaceManager;

class CMissingReferencesWindow final:
  public ui::CDockedWindow
{
public: // Constructor

  CMissingReferencesWindow(const ui::TToolBoxDesc & _Desc, std::shared_ptr<CAssetManager> _AssetManager, std::shared_ptr<const CAssetDatabase> _Assets, std::shared_ptr<CAssetDependencyIndex> _Dependencies, std::shared_ptr<CWorkspaceManager> _WorkspaceManager, std::shared_ptr<core::CActionQueue> _ActionQueue, std::shared_ptr<core::CLog> _Log, ui::CWindowManager & _WindowManager);

private: // CWindow

  void OnGui() override;

private: // Implementation

  void SyncMissingRefs();

  void DrawMissingRef(const TMissingAssetReference & _MissingRef);

  void DrawReferrers(const TMissingAssetReference & _MissingRef) const;

  void HandleRelinkDrop(const TMissingAssetReference & _MissingRef);

  void Relink(const TMissingAssetReference & _MissingRef, const std::string & _ReplacementPath) const;

  std::vector<std::string> FindOpenReferrers(const TMissingAssetReference & _MissingRef) const;

  std::string MakeReferrerPath(const TAssetId & _ReferrerId) const;

  void ReportProblem(core::ELogLevel _Level, const std::string & _Message, std::vector<std::string> _Details = {}) const;

private: // Attributes

  std::shared_ptr<CAssetManager>         m_AssetManager;
  std::shared_ptr<const CAssetDatabase>  m_Assets;
  std::shared_ptr<CAssetDependencyIndex> m_Dependencies;
  std::shared_ptr<CWorkspaceManager>     m_WorkspaceManager;
  std::shared_ptr<core::CActionQueue>    m_ActionQueue;
  std::shared_ptr<core::CLog>            m_Log;
  ui::CWindowManager &                   m_WindowManager;
  ui::CButton                            m_RefreshButton;
  std::vector<TMissingAssetReference>    m_MissingRefs;
  std::uint64_t                          m_AssetsRevision;
  bool                                   m_IsRefreshRequested;
};

} // namespace editor
