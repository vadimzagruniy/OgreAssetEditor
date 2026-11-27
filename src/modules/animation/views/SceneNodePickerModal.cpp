#include "SceneNodePickerModal.h"
#include "editor/timeline/presenter/TimelinePresenter.h"
#include "modules/animation/AnimationEditor.h"
#include "modules/scene/animation/ISceneAnimationHost.h"
#include "modules/scene/model/SceneGraph.h"

#include <imgui.h>

#include <utility>

namespace editor
{

//
// Constructor
//

CSceneNodePickerModal::CSceneNodePickerModal(std::weak_ptr<CAnimationEditor> _Editor):
  CModalWindow("Choose Scene Node", ImVec2(24.0f, 26.0f)),
  m_Editor(std::move(_Editor)),
  m_SelectButtonIndex(AddButton("Select", [this] { OnSelectClicked(); }))
{
  AddButton("Cancel", [this]
  {
    OnCancelClicked();
  });
}

//
// CModalWindow
//

void CSceneNodePickerModal::OnContentGui()
{
  const auto Editor = m_Editor.lock();
  const auto Host   = Editor ? Editor->GetSceneHost() : nullptr;

  if (Host && !Host->GetGraph().HasNode(m_NodeName))
    m_NodeName.clear();

  SetButtonEnabled(m_SelectButtonIndex, Host && !m_NodeName.empty());

  if (!Host)
  {
    ImGui::TextDisabled("The scene is not available.");
    return;
  }

  const auto & Graph = Host->GetGraph();

  if (Graph.GetRootNodes().empty())
  {
    ImGui::TextDisabled("The scene has no nodes.");
    return;
  }

  for (const auto & RootName : Graph.GetRootNodes())
    DrawNode(Graph, RootName);
}

void CSceneNodePickerModal::OnCancel()
{
  if (const auto Editor = m_Editor.lock())
    Editor->GetPresenter().CancelTrackTarget();
}

//
// Implementation
//

void CSceneNodePickerModal::DrawNode(const CSceneGraph & _Graph, const std::string & _NodeName)
{
  const auto & Children = _Graph.GetChildren(_NodeName);

  ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;

  if (Children.empty())
    Flags |= ImGuiTreeNodeFlags_Leaf;

  if (_NodeName == m_NodeName)
    Flags |= ImGuiTreeNodeFlags_Selected;

  const bool IsOpen = ImGui::TreeNodeEx(_NodeName.c_str(), Flags, "%s", _NodeName.c_str());

  if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen())
    m_NodeName = _NodeName;

  if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen())
    OnSelectClicked();

  if (!IsOpen)
    return;

  for (const auto & ChildName : Children)
    DrawNode(_Graph, ChildName);

  ImGui::TreePop();
}

//
// Events
//

void CSceneNodePickerModal::OnSelectClicked()
{
  const auto Editor = m_Editor.lock();
  if (!Editor || m_NodeName.empty())
    return;

  Editor->GetPresenter().ConfirmTrackTarget(m_NodeName);
  Close();
}

void CSceneNodePickerModal::OnCancelClicked()
{
  OnCancel();
  Close();
}

} // namespace editor
