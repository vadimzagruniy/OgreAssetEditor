#include "DockLayoutBuilder.h"
#include "DockSlot.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <algorithm>
#include <type_traits>
#include <utility>

namespace editor::ui
{

static_assert(std::is_same_v<TDockId, ImGuiID>, "TDockId must match ImGuiID");

namespace
{

struct TDockNodes
{
  ImGuiID Center      = 0;
  ImGuiID Left        = 0;
  ImGuiID LeftBottom  = 0;
  ImGuiID Right       = 0;
  ImGuiID RightBottom = 0;
  ImGuiID Bottom      = 0;
};

ImGuiID GetSlotNode(const TDockNodes & _Nodes, EDockSlot _Slot)
{
  switch (_Slot)
  {
    case EDockSlot::Center:      return _Nodes.Center;
    case EDockSlot::Left:        return _Nodes.Left;
    case EDockSlot::LeftBottom:  return _Nodes.LeftBottom;
    case EDockSlot::Right:       return _Nodes.Right;
    case EDockSlot::RightBottom: return _Nodes.RightBottom;
    case EDockSlot::Bottom:      return _Nodes.Bottom;
    case EDockSlot::None:        break;
  }

  return 0;
}

ImGuiID SplitSideBottom(ImGuiID & _SideNode, bool _IsTopUsed, float _BottomHeight)
{
  if (!_IsTopUsed)
    return _SideNode;

  return ImGui::DockBuilderSplitNode(_SideNode, ImGuiDir_Down, _BottomHeight, nullptr, &_SideNode);
}

} // namespace

//
// Constructor
//

CDockLayoutBuilder::CDockLayoutBuilder(const TDockLayoutDesc & _Desc):
  m_Desc(_Desc)
{
  // Empty
}

//
// Interface
//

bool CDockLayoutBuilder::HasLayout(TDockId _DockSpaceId)
{
  const ImGuiDockNode * Node = ImGui::DockBuilderGetNode(_DockSpaceId);
  return Node && Node->IsSplitNode();
}

void CDockLayoutBuilder::DockWindow(std::string _WindowLabel, EDockSlot _Slot)
{
  if (_Slot != EDockSlot::None)
    m_Requests.push_back({std::move(_WindowLabel), _Slot});
}

void CDockLayoutBuilder::Build(TDockId _DockSpaceId) const
{
  ImGui::DockBuilderRemoveNode(_DockSpaceId);
  ImGui::DockBuilderAddNode(_DockSpaceId, ImGuiDockNodeFlags_DockSpace);
  ImGui::DockBuilderSetNodeSize(_DockSpaceId, ImGui::GetMainViewport()->WorkSize);

  const bool IsLeftUsed  = IsSlotUsed(EDockSlot::Left);
  const bool IsRightUsed = IsSlotUsed(EDockSlot::Right);

  TDockNodes Nodes;
  Nodes.Center = _DockSpaceId;

  if (IsRightUsed || IsSlotUsed(EDockSlot::RightBottom))
    Nodes.Right = ImGui::DockBuilderSplitNode(Nodes.Center, ImGuiDir_Right, m_Desc.RightWidth, nullptr, &Nodes.Center);

  if (IsLeftUsed || IsSlotUsed(EDockSlot::LeftBottom))
  {
    const float RemainingWidth = Nodes.Right ? 1.0f - m_Desc.RightWidth : 1.0f;
    const float LeftRatio      = std::clamp(m_Desc.LeftWidth / RemainingWidth, 0.0f, 1.0f);

    Nodes.Left = ImGui::DockBuilderSplitNode(Nodes.Center, ImGuiDir_Left, LeftRatio, nullptr, &Nodes.Center);
  }

  if (IsSlotUsed(EDockSlot::Bottom))
    Nodes.Bottom = ImGui::DockBuilderSplitNode(Nodes.Center, ImGuiDir_Down, m_Desc.BottomHeight, nullptr, &Nodes.Center);

  if (IsSlotUsed(EDockSlot::LeftBottom))
    Nodes.LeftBottom = SplitSideBottom(Nodes.Left, IsLeftUsed, m_Desc.SideBottomHeight);

  if (IsSlotUsed(EDockSlot::RightBottom))
    Nodes.RightBottom = SplitSideBottom(Nodes.Right, IsRightUsed, m_Desc.SideBottomHeight);

  for (const auto & Request : m_Requests)
    ImGui::DockBuilderDockWindow(Request.WindowLabel.c_str(), GetSlotNode(Nodes, Request.Slot));

  ImGui::DockBuilderFinish(_DockSpaceId);
}

//
// Implementation
//

bool CDockLayoutBuilder::IsSlotUsed(EDockSlot _Slot) const
{
  return std::any_of(m_Requests.begin(), m_Requests.end(), [_Slot](const TDockRequest & _Request)
  {
    return _Request.Slot == _Slot;
  });
}

} // namespace editor::ui
