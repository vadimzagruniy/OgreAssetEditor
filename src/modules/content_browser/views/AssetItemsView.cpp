#include "AssetItemsView.h"
#include "editor/assets/AssetFactoryInfo.h"
#include "editor/assets/AssetMenuEntry.h"
#include "editor/assets/AssetPayload.h"
#include "modules/content_browser/presenter/ContentItem.h"
#include "modules/content_browser/presenter/IContentBrowserPresenter.h"
#include "ui/DragPayload.h"

#include <imgui.h>

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <iterator>
#include <utility>

namespace editor
{

namespace
{

const ImU32 FolderIconColor = IM_COL32(214, 172, 84, 255);
const ImU32 FileIconColor   = IM_COL32(104, 136, 178, 255);

ESelectionMode GetSelectionMode()
{
  const ImGuiIO & InputState = ImGui::GetIO();

  if (InputState.KeyShift)
    return ESelectionMode::Range;

  if (InputState.KeyCtrl)
    return ESelectionMode::Toggle;

  return ESelectionMode::Replace;
}

std::string FitTextToWidth(const std::string & _Text, float _Width)
{
  if (ImGui::CalcTextSize(_Text.c_str()).x <= _Width)
    return _Text;

  const char * Ellipsis      = "...";
  const float  EllipsisWidth = ImGui::CalcTextSize(Ellipsis).x;

  std::string Fitted = _Text;

  while (!Fitted.empty() && ImGui::CalcTextSize(Fitted.c_str()).x + EllipsisWidth > _Width)
    Fitted.pop_back();

  return Fitted + Ellipsis;
}

} // namespace

//
// Constructor/Destructor
//

CAssetItemsView::CAssetItemsView(IContentBrowserPresenter & _Presenter):
  m_Presenter(_Presenter),
  m_RenameBuffer{},
  m_IsRenameFocusPending(false)
{
  // Empty
}

CAssetItemsView::~CAssetItemsView() = default;

//
// Interface
//

void CAssetItemsView::Draw()
{
  if (ImGui::BeginChild("##AssetItems"))
  {
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

    DrawItems(m_Presenter.GetItems());

    ImGui::PopStyleVar();

    if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
      m_Presenter.ClearSelection();

    DrawEmptySpaceContextMenu();
    HandleShortcuts();
  }

  ImGui::EndChild();
}

//
// Protected interface
//

void CAssetItemsView::DrawItemButton(const TContentItem & _Item, const ImVec2 & _Position, const ImVec2 & _Size)
{
  ImGui::SetCursorScreenPos(_Position);
  ImGui::InvisibleButton("##Item", _Size);

  if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
  {
    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
      m_Presenter.Open(_Item.RelativePath);
    else
      m_Presenter.Select(_Item.RelativePath, GetSelectionMode());
  }

  const bool IsSelected = m_Presenter.IsSelected(_Item.RelativePath);
  const bool IsHovered  = ImGui::IsItemHovered();

  if (IsSelected || IsHovered)
  {
    const ImGuiCol ColorIndex = IsSelected ? ImGuiCol_Header : ImGuiCol_HeaderHovered;
    const ImVec2   Max(_Position.x + _Size.x, _Position.y + _Size.y);

    ImGui::GetWindowDrawList()->AddRectFilled(_Position, Max, ImGui::GetColorU32(ColorIndex), ImGui::GetStyle().FrameRounding);
  }

  DrawItemContextMenu(_Item);
  DrawItemDragSource(_Item);
}

void CAssetItemsView::DrawItemIcon(const TContentItem & _Item, const ImVec2 & _Position, float _Size) const
{
  auto &       DrawList = *ImGui::GetWindowDrawList();
  const float  Rounding = _Size * 0.08f;
  const ImVec2 Max(_Position.x + _Size, _Position.y + _Size);

  if (!_Item.IsDirectory)
  {
    DrawList.AddRectFilled(ImVec2(_Position.x + _Size * 0.12f, _Position.y), ImVec2(Max.x - _Size * 0.12f, Max.y), FileIconColor, Rounding);
    return;
  }

  const float TabHeight = _Size * 0.15f;

  DrawList.AddRectFilled(_Position, ImVec2(_Position.x + _Size * 0.45f, _Position.y + TabHeight * 2.0f), FolderIconColor, Rounding);
  DrawList.AddRectFilled(ImVec2(_Position.x, _Position.y + TabHeight), Max, FolderIconColor, Rounding);
}

void CAssetItemsView::DrawItemLabel(const TContentItem & _Item, const ImVec2 & _Position, float _Width, bool _IsCentered)
{
  if (IsRenaming(_Item))
  {
    DrawRenameField(_Item, _Position, _Width);
    return;
  }

  const std::string Label      = FitTextToWidth(_Item.Name, _Width);
  const float       LabelWidth = ImGui::CalcTextSize(Label.c_str()).x;
  const float       OffsetX    = _IsCentered ? (_Width - LabelWidth) * 0.5f : 0.0f;

  ImGui::GetWindowDrawList()->AddText(ImVec2(_Position.x + OffsetX, _Position.y), ImGui::GetColorU32(ImGuiCol_Text), Label.c_str());

  if (Label != _Item.Name)
    ImGui::SetItemTooltip("%s", _Item.Name.c_str());
}

bool CAssetItemsView::IsRenaming(const TContentItem & _Item) const
{
  return _Item.RelativePath == m_RenamingPath;
}

//
// Implementation
//

void CAssetItemsView::DrawItemContextMenu(const TContentItem & _Item)
{
  if (!ImGui::BeginPopupContextItem())
    return;

  if (!m_Presenter.IsSelected(_Item.RelativePath))
    m_Presenter.Select(_Item.RelativePath, ESelectionMode::Replace);

  const auto & Path = _Item.RelativePath;

  std::vector<TAssetMenuEntry> Entries
  {
    {"Open",      "Enter",  EAssetMenuGroup::Open,   true, [this, Path] { m_Presenter.Open(Path);               }},
    {"Rename",    "F2",     EAssetMenuGroup::Edit,   true, [this, Path] { BeginRename(Path);                    }},
    {"Duplicate", "Ctrl+D", EAssetMenuGroup::Edit,   true, [this]       { m_Presenter.DuplicateSelection();     }},
    {"Delete",    "Del",    EAssetMenuGroup::Delete, true, [this]       { m_Presenter.RequestDeleteSelection(); }}
  };

  auto ContextActions = m_Presenter.GetContextActions(Path);
  std::move(ContextActions.begin(), ContextActions.end(), std::back_inserter(Entries));

  DrawMenuEntries(std::move(Entries));

  ImGui::EndPopup();
}

void CAssetItemsView::DrawItemDragSource(const TContentItem & _Item) const
{
  if (_Item.IsDirectory || !ImGui::BeginDragDropSource())
    return;

  GetAssetPayload().Set(_Item.RelativePath.generic_string());
  ImGui::TextUnformatted(_Item.Name.c_str());

  ImGui::EndDragDropSource();
}

void CAssetItemsView::DrawMenuEntries(std::vector<TAssetMenuEntry> _Entries)
{
  std::stable_sort(_Entries.begin(), _Entries.end(), [](const TAssetMenuEntry & _Left, const TAssetMenuEntry & _Right)
  {
    return _Left.Group < _Right.Group;
  });

  for (std::size_t Index = 0; Index < _Entries.size(); ++Index)
  {
    const auto & Entry = _Entries[Index];

    if (Index > 0 && Entry.Group != _Entries[Index - 1].Group)
      ImGui::Separator();

    if (ImGui::MenuItem(Entry.Label.c_str(), Entry.Shortcut.c_str(), false, Entry.IsEnabled) && Entry.Execute)
      Entry.Execute();
  }
}

void CAssetItemsView::DrawEmptySpaceContextMenu()
{
  if (!ImGui::BeginPopupContextWindow("##EmptySpaceMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
    return;

  if (ImGui::MenuItem("New Folder"))
    m_Presenter.CreateFolder();

  DrawCreateMenu();

  ImGui::EndPopup();
}

void CAssetItemsView::DrawCreateMenu()
{
  const auto Factories = m_Presenter.GetAssetFactories();
  if (Factories.empty() || !ImGui::BeginMenu("Create"))
    return;

  for (const auto & Factory : Factories)
  {
    if (ImGui::MenuItem(Factory.Label.c_str()))
      m_Presenter.CreateAsset(Factory.Identifier);
  }

  ImGui::EndMenu();
}

void CAssetItemsView::DrawRenameField(const TContentItem & _Item, const ImVec2 & _Position, float _Width)
{
  ImGui::SetCursorScreenPos(_Position);

  if (m_IsRenameFocusPending)
    ImGui::SetKeyboardFocusHere();

  ImGui::SetNextItemWidth(_Width);

  const ImGuiInputTextFlags InputFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;

  if (ImGui::InputText("##Rename", m_RenameBuffer, sizeof(m_RenameBuffer), InputFlags))
  {
    m_Presenter.Rename(_Item.RelativePath, m_RenameBuffer);
    m_RenamingPath.clear();
    return;
  }

  if (!m_IsRenameFocusPending && !ImGui::IsItemActive())
    m_RenamingPath.clear();

  m_IsRenameFocusPending = false;
}

void CAssetItemsView::HandleShortcuts()
{
  if (!m_RenamingPath.empty() || !ImGui::IsWindowFocused())
    return;

  const auto & Selection = m_Presenter.GetSelection();

  if (ImGui::Shortcut(ImGuiKey_Delete) && !Selection.empty())
    m_Presenter.RequestDeleteSelection();

  if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_D) && !Selection.empty())
    m_Presenter.DuplicateSelection();

  if (ImGui::Shortcut(ImGuiKey_F2) && Selection.size() == 1)
    BeginRename(*Selection.begin());

  if (ImGui::Shortcut(ImGuiKey_Enter) && Selection.size() == 1)
    m_Presenter.Open(*Selection.begin());

  if (ImGui::Shortcut(ImGuiKey_Backspace))
    m_Presenter.NavigateUp();
}

void CAssetItemsView::BeginRename(const std::filesystem::path & _Path)
{
  const std::string Name = _Path.filename().string();

  std::snprintf(m_RenameBuffer, sizeof(m_RenameBuffer), "%s", Name.c_str());

  m_RenamingPath         = _Path;
  m_IsRenameFocusPending = true;
}

} // namespace editor
