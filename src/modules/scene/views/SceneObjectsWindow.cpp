#include "SceneObjectsWindow.h"
#include "modules/scene/SceneTemplatePayload.h"
#include "modules/scene/model/SceneNodeTemplate.h"
#include "modules/scene/presenter/ISceneEditorPresenter.h"
#include "ui/DragPayload.h"
#include "ui/ToolBoxDesc.h"

#include <imgui.h>

#include <string>
#include <utility>

namespace editor
{

//
// Constructor
//

CSceneObjectsWindow::CSceneObjectsWindow(const ui::TToolBoxDesc & _Desc, std::shared_ptr<ISceneEditorPresenter> _Presenter):
  CDockedWindow(_Desc.Title, "SceneObjects", _Desc.Visible),
  m_Presenter(std::move(_Presenter))
{
  // Empty
}

//
// CWindow
//

void CSceneObjectsWindow::OnGui()
{
  if (!m_Presenter->GetDocument())
  {
    ImGui::TextUnformatted("No scene is open");
    return;
  }

  std::string Category;

  for (const auto & Template : m_Presenter->GetTemplates())
  {
    if (Template.Category != Category)
    {
      Category = Template.Category;
      ImGui::SeparatorText(Category.c_str());
    }

    DrawTemplate(Template);
  }
}

//
// Implementation
//

void CSceneObjectsWindow::DrawTemplate(const TSceneNodeTemplate & _Template) const
{
  ImGui::PushID(_Template.Identifier.c_str());

  if (ImGui::Selectable(_Template.Label.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
    m_Presenter->CreateFromTemplate(_Template.Identifier, TOutlinerDropTarget{});

  ImGui::SetItemTooltip("Drag into the viewport or the outliner, or double-click to add to the scene root.");

  if (ImGui::BeginDragDropSource())
  {
    GetTemplatePayload().Set(_Template.Identifier);
    ImGui::TextUnformatted(_Template.Label.c_str());
    ImGui::EndDragDropSource();
  }

  ImGui::PopID();
}

} // namespace editor
