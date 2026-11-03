#include "SceneOutlinerWindow.h"
#include "editor/assets/AssetPayload.h"
#include "modules/scene/SceneDocument.h"
#include "modules/scene/SceneInstance.h"
#include "modules/scene/presenter/ISceneEditorPresenter.h"
#include "ui/DragPayload.h"
#include "ui/DropPayload.h"
#include "ui/ToolBoxDesc.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <algorithm>
#include <optional>
#include <utility>

namespace editor
{

namespace
{

const ui::CDragPayload & GetNodePayload()
{
  static const ui::CDragPayload Payload("SCENE_NODE");
  return Payload;
}

constexpr ImGuiTreeNodeFlags NodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth;

const ImVec4 ErrorColour(0.95f, 0.40f, 0.35f, 1.0f);

} // namespace

//
// Constructor
//

CSceneOutlinerWindow::CSceneOutlinerWindow(const ui::TToolBoxDesc & _Desc, std::shared_ptr<ISceneEditorPresenter> _Presenter):
  CDockedWindow(_Desc.Title, "SceneOutliner", _Desc.Visible),
  m_Presenter(std::move(_Presenter)),
  m_RenameBuffer{},
  m_IsRenameFocusPending(false),
  m_IsRevealPending(false)
{
  // Empty
}

//
// CWindow
//

void CSceneOutlinerWindow::OnGui()
{
  const auto Document = m_Presenter->GetDocument();
  if (!Document)
  {
    ImGui::TextUnformatted("No scene is open");
    return;
  }

  const std::string Selection = Document->GetSelection();
  m_IsRevealPending = Selection != m_LastSelection;
  m_LastSelection   = Selection;

  for (const auto & Name : Document->GetInstance().GetRootNodes())
    DrawNode(*Document, Name);

  if (!m_RenameError.empty())
    ImGui::TextColored(ErrorColour, "%s", m_RenameError.c_str());

  DrawEmptySpace(*Document);
  HandleShortcuts(*Document);
}

//
// Implementation
//

void CSceneOutlinerWindow::DrawNode(const CSceneDocument & _Document, const std::string & _Name)
{
  const auto & Node = _Document.GetInstance().GetNode(_Name);

  ImGuiTreeNodeFlags Flags = NodeFlags;
  if (Node.Children.empty())
    Flags |= ImGuiTreeNodeFlags_Leaf;

  if (_Document.GetSelection() == _Name)
    Flags |= ImGuiTreeNodeFlags_Selected;

  ImGui::PushID(_Name.c_str());

  const bool IsSelectionInside = m_IsRevealPending && m_LastSelection != _Name && _Document.GetInstance().IsSameOrDescendant(m_LastSelection, _Name);
  if (IsSelectionInside)
    ImGui::SetNextItemOpen(true);

  const bool IsRenaming = m_RenamingNode == _Name;
  const bool IsOpen     = ImGui::TreeNodeEx("##Node", Flags, "%s", IsRenaming ? "" : _Name.c_str());

  if (m_IsRevealPending && m_LastSelection == _Name)
    ImGui::ScrollToItem(ImGuiScrollFlags_KeepVisibleEdgeY);

  if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
    m_Presenter->Select(_Name);

  if (ImGui::BeginDragDropSource())
  {
    GetNodePayload().Set(_Name);
    ImGui::TextUnformatted(_Name.c_str());
    ImGui::EndDragDropSource();
  }

  AcceptDrop(_Document, _Name);
  DrawNodeContextMenu(_Name);

  if (IsRenaming)
  {
    ImGui::SameLine();
    DrawRenameField(_Document);
  }

  if (IsOpen)
  {
    for (const auto & ChildName : Node.Children)
      DrawNode(_Document, ChildName);

    ImGui::TreePop();
  }

  ImGui::PopID();
}

void CSceneOutlinerWindow::DrawRenameField(const CSceneDocument & _Document)
{
  if (m_IsRenameFocusPending)
    ImGui::SetKeyboardFocusHere();

  ImGui::SetNextItemWidth(-1.0f);

  const ImGuiInputTextFlags InputFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;

  if (ImGui::InputText("##Rename", m_RenameBuffer.data(), m_RenameBuffer.size(), InputFlags))
  {
    const std::string NewName = m_RenameBuffer.data();

    if (!_Document.CanRename(m_RenamingNode, NewName, m_RenameError))
    {
      m_IsRenameFocusPending = true;
      return;
    }

    m_Presenter->RenameNode(m_RenamingNode, NewName);
    m_RenamingNode.clear();
    m_RenameError.clear();
    return;
  }

  if (!m_IsRenameFocusPending && !ImGui::IsItemActive())
  {
    m_RenamingNode.clear();
    m_RenameError.clear();
  }

  m_IsRenameFocusPending = false;
}

void CSceneOutlinerWindow::DrawNodeContextMenu(const std::string & _Name)
{
  if (!ImGui::BeginPopupContextItem("##NodeMenu"))
    return;

  if (ImGui::MenuItem("Create Child"))
    m_Presenter->CreateNode(_Name);

  if (ImGui::MenuItem("Rename", "F2"))
    BeginRename(_Name);

  if (ImGui::MenuItem("Delete", "Del"))
    m_Presenter->DeleteNode(_Name);

  ImGui::EndPopup();
}

void CSceneOutlinerWindow::DrawEmptySpace(const CSceneDocument & _Document)
{
  const ImVec2 Available = ImGui::GetContentRegionAvail();
  ImGui::Dummy(ImVec2(Available.x, std::max(Available.y, ImGui::GetFrameHeight())));

  AcceptDrop(_Document, {});

  if (!ImGui::BeginPopupContextWindow("##EmptySpaceMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
    return;

  if (ImGui::MenuItem("Create Node"))
    m_Presenter->CreateNode({});

  ImGui::EndPopup();
}

void CSceneOutlinerWindow::AcceptDrop(const CSceneDocument & _Document, const std::string & _ParentName) const
{
  const auto                AssetPath = GetAssetPayload().Peek();
  const TOutlinerDropTarget AssetTarget{_ParentName};

  std::optional<std::string> AssetHint;
  if (AssetPath)
  {
    AssetHint = m_Presenter->GetDropHint(*AssetPath, AssetTarget);
    if (!AssetHint)
      return;
  }

  if (!ImGui::BeginDragDropTarget())
    return;

  if (const auto Name = GetNodePayload().Accept(); Name && _Document.CanMove(*Name, _ParentName))
    m_Presenter->MoveNode(*Name, _ParentName);

  if (const auto Payload = GetAssetPayload().AcceptBeforeDelivery(); Payload && AssetHint)
  {
    ImGui::SetTooltip("%s", AssetHint->c_str());

    if (Payload->IsDelivered)
      m_Presenter->DropAsset(Payload->Data, AssetTarget);
  }

  ImGui::EndDragDropTarget();
}

void CSceneOutlinerWindow::HandleShortcuts(const CSceneDocument & _Document)
{
  if (!m_RenamingNode.empty() || !ImGui::IsWindowFocused())
    return;

  const std::string Selection = _Document.GetSelection();
  if (Selection.empty())
    return;

  if (ImGui::IsKeyPressed(ImGuiKey_Delete, false))
    m_Presenter->DeleteNode(Selection);

  if (ImGui::IsKeyPressed(ImGuiKey_F2, false))
    BeginRename(Selection);
}

void CSceneOutlinerWindow::BeginRename(const std::string & _Name)
{
  m_RenamingNode = _Name;
  m_RenameError.clear();
  m_IsRenameFocusPending = true;

  const auto Length = std::min(_Name.size(), m_RenameBuffer.size() - 1);
  std::copy_n(_Name.begin(), Length, m_RenameBuffer.begin());
  m_RenameBuffer[Length] = '\0';
}

} // namespace editor
