#pragma once

#include "core/TypeKey.h"

#include <memory>
#include <string>
#include <vector>

namespace editor
{

namespace core
{
class CEditorService;
} // namespace core

class CAssetDocument;
class IAttachedEditor;
class IPropertySource;

class CEditorWorkspace
{
public: // Constructor/Destructor

  explicit CEditorWorkspace(std::shared_ptr<CAssetDocument> _Document);

  virtual ~CEditorWorkspace();

  CEditorWorkspace(const CEditorWorkspace &) = delete;

  CEditorWorkspace & operator=(const CEditorWorkspace &) = delete;

public: // Interface

  CAssetDocument & GetDocument() const noexcept;

  std::string GetTitle() const;

  bool IsDirty() const;

  bool Save(std::string & _Error);

  void Activate();

  void Deactivate();

  bool IsActive() const noexcept;

  core::CEditorService & GetServices() const noexcept;

  void AttachEditor(std::shared_ptr<IAttachedEditor> _Editor);

  void DetachEditor(const IAttachedEditor & _Editor);

  const std::vector<std::shared_ptr<IAttachedEditor>> & GetAttachedEditors() const noexcept;

  int GetFocusFrame() const noexcept;

  void SetFocusFrame(int _Frame) noexcept;

  virtual std::vector<core::TTypeKey> GetToolBoxes() const;

  virtual std::shared_ptr<IPropertySource> GetInspected() const;

public: // Events

  virtual void OnActivated();

  virtual void OnDeactivated();

  virtual void OnGui() = 0;

private: // Attributes

  std::shared_ptr<CAssetDocument>               m_Document;
  std::unique_ptr<core::CEditorService>         m_Services;
  std::vector<std::shared_ptr<IAttachedEditor>> m_AttachedEditors;
  int                                           m_FocusFrame;
  bool                                          m_IsActive;
};

} // namespace editor
