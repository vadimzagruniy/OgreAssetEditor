#include "MaterialWorkspace.h"
#include "MaterialDocument.h"
#include "MaterialEditingSession.h"
#include "MaterialInstance.h"
#include "editor/render/PreviewScene.h"
#include "editor/render/PreviewShape.h"
#include "editor/render/PreviewViewport.h"
#include "views/MaterialInspectorWindow.h"
#include "views/MaterialOutlineWindow.h"

#include <imgui.h>

#include <array>
#include <cstddef>
#include <utility>

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

const char * GetShapeLabel(EPreviewShape _Shape)
{
  for (const auto & Option : ShapeOptions)
  {
    if (Option.Shape == _Shape)
      return Option.Label;
  }

  return ShapeOptions.front().Label;
}

} // namespace

//
// Constructor/Destructor
//

CMaterialWorkspace::CMaterialWorkspace(std::unique_ptr<CMaterialInstance> _Instance, std::shared_ptr<CMaterialEditingSession> _Session):
  CEditorWorkspace(std::move(_Instance)),
  m_Session(std::move(_Session)),
  m_Document(std::make_shared<CMaterialDocument>(*static_cast<CMaterialInstance *>(GetInstance()), GetUndoStack())),
  m_Preview(std::make_unique<CPreviewScene>()),
  m_Shape(EPreviewShape::Sphere),
  m_PreviewMaterialIndex(-1)
{
  m_Preview->SetShape(m_Shape);
  UpdatePreviewMaterial();
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
  m_Session->SetDocument(m_Document);
  m_Preview->SetActive(true);
}

void CMaterialWorkspace::OnDeactivated()
{
  m_Document->CommitPreview();
  m_Session->ResetDocument(*m_Document);
  m_Preview->SetActive(false);
}

void CMaterialWorkspace::OnGui()
{
  UpdatePreviewMaterial();

  DrawToolbar();
  DrawPreviewViewport("##MaterialViewport", *m_Preview);
}

bool CMaterialWorkspace::OnSave(std::string & _Error)
{
  m_Document->CommitPreview();

  return static_cast<CMaterialInstance *>(GetInstance())->Save(_Error);
}

//
// Implementation
//

void CMaterialWorkspace::DrawToolbar()
{
  ImGui::SetNextItemWidth(ImGui::GetFontSize() * 8.0f);

  if (ImGui::BeginCombo("Shape", GetShapeLabel(m_Shape)))
  {
    for (const auto & Option : ShapeOptions)
    {
      if (ImGui::Selectable(Option.Label, Option.Shape == m_Shape) && Option.Shape != m_Shape)
      {
        m_Shape = Option.Shape;
        m_Preview->SetShape(m_Shape);
      }
    }

    ImGui::EndCombo();
  }

  if (m_Document->GetInstance().HasExpandedFeatures())
  {
    ImGui::SameLine();
    ImGui::TextColored(WarningColour, "Script uses inheritance, variables or imports: saving writes every material in full.");
  }
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
