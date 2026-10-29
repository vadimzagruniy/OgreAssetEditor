#include "WindowManager.h"
#include "ModalWindow.h"
#include "ToolBoxDesc.h"
#include "Window.h"

#include <imgui.h>

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

  m_Windows.push_back({_Desc.Module, _Window});
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
  for (const auto & WindowEntry : m_Windows)
    WindowEntry.Window->Draw();

  for (auto ModalIterator = m_Modals.begin(); ModalIterator != m_Modals.end();)
  {
    if ((*ModalIterator)->Draw())
      ++ModalIterator;
    else
      ModalIterator = m_Modals.erase(ModalIterator);
  }
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

} // namespace editor::ui
