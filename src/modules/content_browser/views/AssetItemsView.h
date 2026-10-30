#pragma once

#include <filesystem>
#include <string>
#include <vector>

struct ImVec2;

namespace editor
{

class IContentBrowserPresenter;
struct TContentItem;

class CAssetItemsView
{
public: // Destructor

  virtual ~CAssetItemsView();

public: // Interface

  void Draw();

protected: // Constructor

  explicit CAssetItemsView(IContentBrowserPresenter & _Presenter);

protected: // Interface

  virtual void DrawItems(const std::vector<TContentItem> & _Items) = 0;

  void DrawItemButton(const TContentItem & _Item, const ImVec2 & _Position, const ImVec2 & _Size);

  void DrawItemIcon(const TContentItem & _Item, const ImVec2 & _Position, float _Size) const;

  void DrawItemLabel(const TContentItem & _Item, const ImVec2 & _Position, float _Width, bool _IsCentered);

  bool IsRenaming(const TContentItem & _Item) const;

private: // Implementation

  void DrawItemContextMenu(const TContentItem & _Item);

  void DrawEmptySpaceContextMenu();

  void DrawRenameField(const TContentItem & _Item, const ImVec2 & _Position, float _Width);

  void HandleShortcuts();

  void BeginRename(const std::filesystem::path & _Path);

private: // Attributes

  IContentBrowserPresenter & m_Presenter;
  std::filesystem::path      m_RenamingPath;
  char                       m_RenameBuffer[256];
  bool                       m_IsRenameFocusPending;
};

} // namespace editor
