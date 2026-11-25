#include "PropertiesWindow.h"
#include "editor/assets/AssetRecord.h"
#include "editor/documents/AssetDocument.h"
#include "editor/documents/AssetProperty.h"
#include "editor/inspector/controls/PropertyPanel.h"
#include "editor/inspector/IPropertySource.h"
#include "editor/workspaces/EditorWorkspace.h"
#include "editor/workspaces/WorkspaceController.h"
#include "editor/workspaces/WorkspaceManager.h"
#include "ui/ToolBoxDesc.h"

#include <imgui.h>

#include <utility>

namespace editor
{

//
// Constructor/Destructor
//

CPropertiesWindow::CPropertiesWindow(const ui::TToolBoxDesc & _Desc, TPropertyControlContext _Context, std::shared_ptr<CWorkspaceController> _WorkspaceController):
  CDockedWindow(_Desc.Title, "Properties", _Desc.Visible),
  m_WorkspaceManager(_Context.WorkspaceManager),
  m_WorkspaceController(std::move(_WorkspaceController)),
  m_Panel(std::make_unique<CPropertyPanel>(std::move(_Context)))
{
  // Empty
}

CPropertiesWindow::~CPropertiesWindow() = default;

//
// CWindow
//

void CPropertiesWindow::OnGui()
{
  const auto * Workspace = m_WorkspaceManager->GetActive();
  if (!Workspace)
  {
    ImGui::TextUnformatted("No asset selected");
    return;
  }

  if (const auto Source = m_WorkspaceController->GetInspected())
  {
    DrawSource(*Source);
    return;
  }

  DrawAssetInfo(m_WorkspaceController->GetFocusedDocument(*Workspace));
}

//
// Implementation
//

void CPropertiesWindow::DrawSource(IPropertySource & _Source)
{
  ImGui::TextUnformatted(_Source.GetTitle().c_str());
  ImGui::Separator();

  m_Panel->Draw(_Source);
}

void CPropertiesWindow::DrawAssetInfo(const CAssetDocument & _Document) const
{
  ImGui::Text("Type: %s", _Document.GetTypeName().c_str());
  ImGui::Text("Path: %s", _Document.GetRecord().RelativePath.generic_string().c_str());

  ImGui::Separator();

  for (const auto & Property : _Document.GetProperties())
    ImGui::Text("%s: %s", Property.Name.c_str(), Property.Value.c_str());
}

} // namespace editor
