#include "FolderTreeView.h"
#include "editor/assets/AssetNode.h"
#include "editor/assets/AssetPath.h"
#include "modules/content_browser/presenter/IContentBrowserPresenter.h"

#include <imgui.h>

#include <algorithm>

namespace editor
{

namespace
{

bool HasSubfolders(const TAssetNode & _Folder)
{
  return std::any_of(_Folder.Children.begin(), _Folder.Children.end(), [](const TAssetNode & _Child)
  {
    return _Child.IsDirectory;
  });
}

} // namespace

//
// Constructor
//

CFolderTreeView::CFolderTreeView(IContentBrowserPresenter & _Presenter):
  m_Presenter(_Presenter)
{
  // Empty
}

//
// Interface
//

void CFolderTreeView::Draw()
{
  const bool IsRevealPending = m_RevealedFolder != m_Presenter.GetCurrentFolder();

  DrawFolder(m_Presenter.GetRootFolder(), IsRevealPending);

  m_RevealedFolder = m_Presenter.GetCurrentFolder();
}

//
// Implementation
//

void CFolderTreeView::DrawFolder(const TAssetNode & _Folder, bool _IsRevealPending)
{
  const auto & FolderPath    = _Folder.Record.RelativePath;
  const auto & CurrentFolder = m_Presenter.GetCurrentFolder();

  ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

  if (FolderPath.empty())
    Flags |= ImGuiTreeNodeFlags_DefaultOpen;

  if (FolderPath == CurrentFolder)
    Flags |= ImGuiTreeNodeFlags_Selected;

  if (!HasSubfolders(_Folder))
    Flags |= ImGuiTreeNodeFlags_Leaf;

  if (_IsRevealPending && FolderPath != CurrentFolder && IsSameOrInside(CurrentFolder, FolderPath))
    ImGui::SetNextItemOpen(true);

  const bool IsOpen = ImGui::TreeNodeEx(_Folder.Name.c_str(), Flags);

  if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen())
    m_Presenter.NavigateTo(FolderPath);

  if (ImGui::BeginPopupContextItem())
  {
    if (ImGui::MenuItem("New Folder"))
    {
      m_Presenter.NavigateTo(FolderPath);
      m_Presenter.CreateFolder();
    }

    ImGui::EndPopup();
  }

  if (!IsOpen)
    return;

  for (const auto & Child : _Folder.Children)
  {
    if (Child.IsDirectory)
      DrawFolder(Child, _IsRevealPending);
  }

  ImGui::TreePop();
}

} // namespace editor
