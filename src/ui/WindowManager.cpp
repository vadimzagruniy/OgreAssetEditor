#include "WindowManager.h"
#include "ModalWindow.h"
#include "ToolBoxDesc.h"
#include "Window.h"
#include "ui/dock_layout/DockLayoutBuilder.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <algorithm>
#include <typeinfo>

namespace editor::ui
{

//
// Constructor/Destructor
//

CWindowManager::CWindowManager()
{
  // Empty
}

CWindowManager::~CWindowManager() = default;

//
// Interface
//

bool CWindowManager::RegisterToolBox(const TToolBoxDesc & _Desc, std::shared_ptr<CWindow> _Window)
{
  if (!_Window)
    return false;

  const core::TTypeKey ToolBoxType = typeid(*_Window);
  if (m_ToolBoxes.find(ToolBoxType) != m_ToolBoxes.end())
    return false;

  _Window->SetVisible(_Desc.Visible);
  _Window->SetTitleBarHidden(_Desc.IsTitleBarHidden);

  const auto InsertPosition = std::upper_bound(m_Windows.begin(), m_Windows.end(), _Desc.DockOrder, [](int _DockOrder, const TWindowEntry & _Entry)
  {
    return _DockOrder < _Entry.DockOrder;
  });

  m_Windows.insert(InsertPosition, {_Desc.Module, _Window, _Desc.DockSlot, _Desc.DockOrder});
  m_ToolBoxes.emplace(ToolBoxType, std::move(_Window));

  return true;
}

std::shared_ptr<CWindow> CWindowManager::GetToolBox(const core::TTypeKey & _ToolBoxType) const
{
  const auto WindowEntry = m_ToolBoxes.find(_ToolBoxType);
  return WindowEntry == m_ToolBoxes.end() ? nullptr : WindowEntry->second;
}

bool CWindowManager::ShowToolBox(const core::TTypeKey & _ToolBoxType)
{
  const auto ToolBox = GetToolBox(_ToolBoxType);
  if (!ToolBox)
    return false;

  ToolBox->SetVisible(true);

  return true;
}

bool CWindowManager::HideToolBox(const core::TTypeKey & _ToolBoxType)
{
  const auto ToolBox = GetToolBox(_ToolBoxType);
  if (!ToolBox)
    return false;

  ToolBox->SetVisible(false);

  return true;
}

void CWindowManager::UnregisterModuleToolBoxes(const std::string & _Module)
{
  for (auto WindowIterator = m_Windows.begin(); WindowIterator != m_Windows.end();)
  {
    if (WindowIterator->Module != _Module)
    {
      ++WindowIterator;
      continue;
    }

    for (auto ToolBoxIterator = m_ToolBoxes.begin(); ToolBoxIterator != m_ToolBoxes.end();)
    {
      if (ToolBoxIterator->second == WindowIterator->Window)
        ToolBoxIterator = m_ToolBoxes.erase(ToolBoxIterator);
      else
        ++ToolBoxIterator;
    }

    WindowIterator = m_Windows.erase(WindowIterator);
  }
}

void CWindowManager::OpenModal(std::unique_ptr<CModalWindow> _Modal)
{
  if (_Modal)
    m_Modals.push_back(std::move(_Modal));
}

void CWindowManager::Draw()
{
  for (auto & WindowEntry : m_Windows)
  {
    if (!WindowEntry.IsPlacementChecked && WindowEntry.Window->IsVisible())
      PlaceNewWindow(WindowEntry);

    WindowEntry.Window->Draw();
  }

  if (!m_Modals.empty() && !m_Modals.front()->Draw())
    m_Modals.pop_front();
}

void CWindowManager::DrawMenuItems()
{
  for (const auto & WindowEntry : m_Windows)
  {
    bool IsVisible = WindowEntry.Window->IsVisible();
    if (ImGui::MenuItem(WindowEntry.Window->GetTitle().c_str(), nullptr, &IsVisible))
      WindowEntry.Window->SetVisible(IsVisible);
  }
}

void CWindowManager::AddDefaultDocking(CDockLayoutBuilder & _Builder) const
{
  for (const auto & WindowEntry : m_Windows)
    _Builder.DockWindow(WindowEntry.Window->GetLabel(), WindowEntry.DockSlot);
}

//
// Implementation
//

void CWindowManager::PlaceNewWindow(TWindowEntry & _Entry) const
{
  _Entry.IsPlacementChecked = true;

  if (_Entry.DockSlot == EDockSlot::None || ImGui::FindWindowSettingsByID(ImHashStr(_Entry.Window->GetLabel().c_str())))
    return;

  if (const unsigned int DockId = FindSlotDockId(_Entry); DockId != 0)
    ImGui::SetNextWindowDockID(DockId, ImGuiCond_FirstUseEver);
}

unsigned int CWindowManager::FindSlotDockId(const TWindowEntry & _Entry) const
{
  for (const auto & WindowEntry : m_Windows)
  {
    if (WindowEntry.DockSlot != _Entry.DockSlot || WindowEntry.Window == _Entry.Window)
      continue;

    const char * Label = WindowEntry.Window->GetLabel().c_str();

    if (const auto * Window = ImGui::FindWindowByName(Label); Window && Window->DockId != 0)
      return Window->DockId;

    if (const auto * Settings = ImGui::FindWindowSettingsByID(ImHashStr(Label)); Settings && Settings->DockId != 0)
      return Settings->DockId;
  }

  return 0;
}

} // namespace editor::ui
