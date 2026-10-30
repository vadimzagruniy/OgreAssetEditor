#pragma once

#include "WorkspaceContext.h"
#include "core/TypeKey.h"
#include "core/UndoStack.h"

#include <memory>
#include <string>
#include <vector>

namespace editor
{

class CAssetInstance;

class CEditorWorkspace:
  public IWorkspaceContext
{
public: // Constructor/Destructor

  explicit CEditorWorkspace(std::unique_ptr<CAssetInstance> _Instance);

  ~CEditorWorkspace() override;

public: // Interface

  CAssetInstance * GetInstance() const noexcept;

  core::CUndoStack & GetUndoStack() noexcept;

  const std::vector<std::string> & GetSelection() const noexcept;

  void SetSelection(std::vector<std::string> _Selection);

  std::string GetTitle() const;

  virtual bool IsDirty() const;

  bool Save(std::string & _Error);

  virtual std::vector<core::TTypeKey> GetToolBoxes() const;

public: // IWorkspaceContext

  CEditorWorkspace * GetWorkspace() const noexcept override;

  CAssetInstance * GetAssetInstance() const noexcept override;

public: // Events

  virtual void OnActivated();

  virtual void OnDeactivated();

  virtual void OnGui() = 0;

protected: // Events

  virtual bool OnSave(std::string & _Error);

private: // Attributes

  std::unique_ptr<CAssetInstance> m_Instance;
  core::CUndoStack                m_UndoStack;
  std::vector<std::string>        m_Selection;
};

} // namespace editor
