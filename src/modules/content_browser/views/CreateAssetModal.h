#pragma once

#include "modules/content_browser/presenter/AssetCreationRequest.h"
#include "ui/ModalWindow.h"
#include "ui/elements/InputText.h"

#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>

namespace editor
{

class IContentBrowserPresenter;
struct TAssetNode;

class CCreateAssetModal final:
  public ui::CModalWindow
{
public: // Constructor

  CCreateAssetModal(std::weak_ptr<IContentBrowserPresenter> _Presenter, TAssetCreationRequest _Request);

protected: // CModalWindow

  void OnContentGui() override;

  void OnCancel() override;

private: // Implementation

  void DrawFolderPicker();

  void DrawFolder(const TAssetNode & _Folder);

  void DrawNameInput();

  void ValidateName();

  void DrawNameError() const;

private: // Events

  void OnCreateClicked();

  void OnCancelClicked();

private: // Attributes

  std::weak_ptr<IContentBrowserPresenter> m_Presenter;
  TAssetCreationRequest                   m_Request;
  std::filesystem::path                   m_Folder;
  ui::CInputText                          m_NameInput;
  std::size_t                             m_CreateButtonIndex;
  std::string                             m_NameError;
  bool                                    m_IsNameValid;
  bool                                    m_IsFocusRequested;
};

} // namespace editor
