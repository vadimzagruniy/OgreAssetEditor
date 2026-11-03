#include "PropertiesWindow.h"
#include "controls/AssetPropertyControl.h"
#include "controls/BoolPropertyControl.h"
#include "controls/ColorPropertyControl.h"
#include "controls/EnumPropertyControl.h"
#include "controls/FloatPropertyControl.h"
#include "controls/GroupPropertyControl.h"
#include "controls/RotationPropertyControl.h"
#include "controls/TextPropertyControl.h"
#include "controls/Vector3PropertyControl.h"
#include "editor/assets/AssetRecord.h"
#include "editor/inspector/IPropertySource.h"
#include "editor/inspector/PropertySchema.h"
#include "editor/workspaces/AssetInstance.h"
#include "editor/workspaces/AssetProperty.h"
#include "editor/workspaces/EditorWorkspace.h"
#include "editor/workspaces/WorkspaceManager.h"
#include "ui/ToolBoxDesc.h"

#include <imgui.h>

#include <utility>

namespace editor
{

//
// Constructor/Destructor
//

CPropertiesWindow::CPropertiesWindow(const ui::TToolBoxDesc & _Desc, TPropertyControlContext _Context):
  CDockedWindow(_Desc.Title, "Properties", _Desc.Visible),
  m_Context(std::move(_Context))
{
  // Empty
}

CPropertiesWindow::~CPropertiesWindow() = default;

//
// CWindow
//

void CPropertiesWindow::OnGui()
{
  const auto * Workspace = m_Context.WorkspaceManager->GetActive();
  if (!Workspace || !Workspace->GetInstance())
  {
    ImGui::TextUnformatted("No asset selected");
    return;
  }

  if (const auto Source = Workspace->GetInspected())
  {
    DrawSource(*Source);
    return;
  }

  DrawAssetInfo(*Workspace->GetInstance());
}

//
// Implementation
//

void CPropertiesWindow::DrawSource(IPropertySource & _Source)
{
  const auto & Schema = _Source.GetSchema();
  if (Schema.Identifier != m_SchemaIdentifier)
  {
    m_SchemaIdentifier = Schema.Identifier;
    m_Controls         = CreateControls(Schema.Properties);
  }

  ImGui::TextUnformatted(_Source.GetTitle().c_str());
  ImGui::Separator();

  for (const auto & Control : m_Controls)
    Control->Draw(_Source);
}

void CPropertiesWindow::DrawAssetInfo(const CAssetInstance & _Instance) const
{
  ImGui::Text("Type: %s", _Instance.GetTypeName().c_str());
  ImGui::Text("Path: %s", _Instance.GetRecord().RelativePath.generic_string().c_str());

  ImGui::Separator();

  for (const auto & Property : _Instance.GetProperties())
    ImGui::Text("%s: %s", Property.Name.c_str(), Property.Value.c_str());
}

std::vector<std::unique_ptr<IPropertyControl>> CPropertiesWindow::CreateControls(const std::vector<TPropertyDesc> & _Descs) const
{
  std::vector<std::unique_ptr<IPropertyControl>> Controls;
  Controls.reserve(_Descs.size());

  for (const auto & Desc : _Descs)
    Controls.push_back(CreateControl(Desc));

  return Controls;
}

std::unique_ptr<IPropertyControl> CPropertiesWindow::CreateControl(const TPropertyDesc & _Desc) const
{
  switch (_Desc.Type)
  {
    case EPropertyType::Group:    return std::make_unique<CGroupPropertyControl>(_Desc.Label, CreateControls(_Desc.Children));
    case EPropertyType::Bool:     return std::make_unique<CBoolPropertyControl>(_Desc);
    case EPropertyType::Float:    return std::make_unique<CFloatPropertyControl>(_Desc);
    case EPropertyType::Enum:     return std::make_unique<CEnumPropertyControl>(_Desc);
    case EPropertyType::Text:     return std::make_unique<CTextPropertyControl>(_Desc);
    case EPropertyType::Asset:    return std::make_unique<CAssetPropertyControl>(_Desc, m_Context);
    case EPropertyType::Vector3:  return std::make_unique<CVector3PropertyControl>(_Desc);
    case EPropertyType::Rotation: return std::make_unique<CRotationPropertyControl>(_Desc);
    case EPropertyType::Color:    return std::make_unique<CColorPropertyControl>(_Desc);
  }

  return std::make_unique<CTextPropertyControl>(_Desc);
}

} // namespace editor
