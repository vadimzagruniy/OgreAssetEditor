#pragma once

#include <filesystem>

namespace editor
{

class IContentBrowserPresenter;
struct TAssetNode;

class CFolderTreeView
{
public: // Constructor

  explicit CFolderTreeView(IContentBrowserPresenter & _Presenter);

public: // Interface

  void Draw();

private: // Implementation

  void DrawFolder(const TAssetNode & _Folder, bool _IsRevealPending);

private: // Attributes

  IContentBrowserPresenter & m_Presenter;
  std::filesystem::path      m_RevealedFolder;
};

} // namespace editor
