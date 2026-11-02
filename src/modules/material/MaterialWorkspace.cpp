#include "MaterialWorkspace.h"
#include "MaterialDocument.h"
#include "MaterialInstance.h"
#include "editor/render/PreviewScene.h"
#include "editor/render/PreviewShape.h"
#include "editor/viewport/Viewport.h"
#include "presenter/MaterialEditorController.h"
#include "views/MaterialInspectorWindow.h"
#include "views/MaterialOutlineWindow.h"
#include "ui/elements/ComboBox.h"
#include "ui/elements/Text.h"

#include <imgui.h>

#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace editor
{

namespace
{

struct TShapeOption
{
  EPreviewShape Shape;
  const char *  Label;
};

constexpr std::array<TShapeOption, 3> ShapeOptions{{
  {EPreviewShape::Sphere, "Sphere"},
  {EPreviewShape::Cube,   "Cube"  },
  {EPreviewShape::Plane,  "Plane" }
}};

const ImVec4 WarningColour(0.95f, 0.75f, 0.30f, 1.0f);

std::optional<std::size_t> FindShapeIndex(EPreviewShape _Shape)
{
  for (std::size_t OptionIndex = 0; OptionIndex < ShapeOptions.size(); ++OptionIndex)
  {
    if (ShapeOptions[OptionIndex].Shape == _Shape)
      return OptionIndex;
  }

  return std::nullopt;
}

} // namespace

//
// Constructor/Destructor
//

CMaterialWorkspace::CMaterialWorkspace(std::unique_ptr<CMaterialInstance> _Instance, std::shared_ptr<CMaterialEditorController> _Controller):
  CEditorWorkspace(std::move(_Instance)),
  m_Controller(std::move(_Controller)),
  m_Document(std::make_shared<CMaterialDocument>(*static_cast<CMaterialInstance *>(GetInstance()), GetUndoStack())),
  m_Preview(std::make_unique<CPreviewScene>()),
  m_Viewport(std::make_unique<CViewport>(*m_Preview)),
  m_Shape(EPreviewShape::Sphere),
  m_PreviewMaterialIndex(-1),
  m_Toolbar(ui::ELayoutDirection::Row),
  m_ShapeCombo(m_Toolbar.Add(ui::CComboBox("Shape"))),
  m_WarningText(m_Toolbar.Add(ui::CText("Script uses inheritance, variables or imports: saving writes every material in full.")))
{
  m_Preview->SetShape(m_Shape);
  UpdatePreviewMaterial();
  SetupToolbar();
}

CMaterialWorkspace::~CMaterialWorkspace() = default;

//
// CEditorWorkspace
//

std::vector<core::TTypeKey> CMaterialWorkspace::GetToolBoxes() const
{
  return {core::TypeOf<CMaterialOutlineWindow>(), core::TypeOf<CMaterialInspectorWindow>()};
}

void CMaterialWorkspace::OnActivated()
{
  m_Controller->SetDocument(m_Document);
  m_Preview->SetActive(true);
}

void CMaterialWorkspace::OnDeactivated()
{
  m_Document->CommitPreview();
  m_Controller->ResetDocument(*m_Document);
  m_Preview->SetActive(false);
}

void CMaterialWorkspace::OnGui()
{
  UpdatePreviewMaterial();

  DrawToolbar();
  m_Viewport->Draw("##MaterialViewport");
}

bool CMaterialWorkspace::OnSave(std::string & _Error)
{
  m_Document->CommitPreview();

  return static_cast<CMaterialInstance *>(GetInstance())->Save(_Error);
}

//
// Implementation
//

void CMaterialWorkspace::SetupToolbar()
{
  std::vector<std::string> ShapeLabels;
  ShapeLabels.reserve(ShapeOptions.size());

  for (const auto & Option : ShapeOptions)
    ShapeLabels.emplace_back(Option.Label);

  m_ShapeCombo.SetItems(std::move(ShapeLabels)).SetSelectedIndex(FindShapeIndex(m_Shape)).OnSelected([this](std::size_t _OptionIndex)
  {
    m_Shape = ShapeOptions[_OptionIndex].Shape;
    m_Preview->SetShape(m_Shape);
  });

  m_WarningText.SetColor(ImGuiCol_Text, WarningColour);
}

void CMaterialWorkspace::DrawToolbar()
{
  m_ShapeCombo.SetWidth(ImGui::GetFontSize() * 8.0f);
  m_WarningText.SetVisible(m_Document->GetInstance().HasExpandedFeatures());

  m_Toolbar.Draw();
}

void CMaterialWorkspace::UpdatePreviewMaterial()
{
  const int MaterialIndex = m_Document->GetSelection().MaterialIndex;
  if (MaterialIndex < 0 || MaterialIndex == m_PreviewMaterialIndex)
    return;

  m_PreviewMaterialIndex = MaterialIndex;
  m_Preview->SetMaterial(m_Document->GetInstance().GetWorkingCopy(static_cast<std::size_t>(MaterialIndex)));
}

} // namespace editor
