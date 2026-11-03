#pragma once

#include "editor/inspector/IPropertySource.h"

#include <memory>
#include <string>

namespace editor
{

class CMaterialDocument;
class IMaterialEditorPresenter;

class CMaterialPropertySource:
  public IPropertySource
{
protected: // Constructor

  explicit CMaterialPropertySource(std::shared_ptr<IMaterialEditorPresenter> _Presenter);

public: // IPropertySource

  std::string GetTitle() const override;

  void Commit() override;

protected: // Service

  std::shared_ptr<const CMaterialDocument> GetDocument() const;

  IMaterialEditorPresenter & GetPresenter() const noexcept;

private: // Attributes

  std::shared_ptr<IMaterialEditorPresenter> m_Presenter;
};

} // namespace editor
