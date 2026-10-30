#pragma once

#include "core/TypeKey.h"

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace editor
{

namespace core
{
class CLog;
} // namespace core

namespace ui
{
class CWindowManager;
} // namespace ui

class CWorkspaceManager;

class CWorkspaceController
{
public: // Constructor/Destructor

  CWorkspaceController(std::shared_ptr<CWorkspaceManager> _WorkspaceManager, std::shared_ptr<ui::CWindowManager> _WindowManager, std::shared_ptr<core::CLog> _Log);

  ~CWorkspaceController();

  CWorkspaceController(const CWorkspaceController &) = delete;

  CWorkspaceController & operator=(const CWorkspaceController &) = delete;

public: // Interface

  bool CanSaveActive() const;

  void SaveActive();

  std::string GetUndoName() const;

  std::string GetRedoName() const;

  void UndoActive();

  void RedoActive();

  void RequestClose(std::size_t _Index);

  void RequestCloseActive();

  void ConfirmUnsavedChanges(std::function<void()> _OnConfirmed);

private: // Implementation

  void UpdateToolBoxes();

private: // Attributes

  std::shared_ptr<CWorkspaceManager>  m_WorkspaceManager;
  std::shared_ptr<ui::CWindowManager> m_WindowManager;
  std::shared_ptr<core::CLog>         m_Log;
  std::vector<core::TTypeKey>         m_ShownToolBoxes;
  std::size_t                         m_ActiveChangedConnection;
};

} // namespace editor
