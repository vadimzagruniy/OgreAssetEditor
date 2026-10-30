#include "MaterialOutlineWindow.h"
#include "modules/material/MaterialDocument.h"
#include "modules/material/MaterialEditingSession.h"
#include "modules/material/MaterialInstance.h"
#include "modules/material/MaterialNode.h"
#include "ui/ToolBoxDesc.h"

#include <OgreMaterial.h>
#include <OgrePass.h>
#include <OgreTechnique.h>
#include <OgreTextureUnitState.h>

#include <imgui.h>

#include <cstddef>
#include <string>
#include <utility>

namespace editor
{

namespace
{

constexpr ImGuiTreeNodeFlags BranchFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
constexpr ImGuiTreeNodeFlags LeafFlags   = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth;

std::string MakeLabel(const char * _Kind, std::size_t _Index, const std::string & _Name)
{
  std::string Label = std::string(_Kind) + " " + std::to_string(_Index);

  if (!_Name.empty() && _Name != std::to_string(_Index))
    Label += ": " + _Name;

  return Label;
}

bool DrawNode(CMaterialDocument & _Document, const TMaterialNode & _Node, const std::string & _Label, ImGuiTreeNodeFlags _Flags)
{
  if (_Document.GetSelection() == _Node)
    _Flags |= ImGuiTreeNodeFlags_Selected;

  const bool IsOpen = ImGui::TreeNodeEx(_Label.c_str(), _Flags);

  if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
    _Document.Select(_Node);

  return IsOpen;
}

} // namespace

//
// Constructor
//

CMaterialOutlineWindow::CMaterialOutlineWindow(const ui::TToolBoxDesc & _Desc, std::shared_ptr<CMaterialEditingSession> _Session):
  CDockedWindow(_Desc.Title, "MaterialOutline", _Desc.Visible),
  m_Session(std::move(_Session))
{
  // Empty
}

//
// CWindow
//

void CMaterialOutlineWindow::OnGui()
{
  const auto Document = m_Session->GetDocument();
  if (!Document)
  {
    ImGui::TextUnformatted("No material is open");
    return;
  }

  const auto MaterialCount = static_cast<int>(Document->GetInstance().GetMaterialCount());

  for (int MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
  {
    ImGui::PushID(MaterialIndex);
    DrawMaterial(*Document, MaterialIndex);
    ImGui::PopID();
  }
}

//
// Implementation
//

void CMaterialOutlineWindow::DrawMaterial(CMaterialDocument & _Document, int _MaterialIndex)
{
  const auto & Instance      = _Document.GetInstance();
  const auto   MaterialIndex = static_cast<std::size_t>(_MaterialIndex);
  const auto & Material      = Instance.GetWorkingCopy(MaterialIndex);

  if (!DrawNode(_Document, TMaterialNode{_MaterialIndex}, Instance.GetMaterialName(MaterialIndex), BranchFlags))
    return;

  for (std::size_t TechniqueIndex = 0; TechniqueIndex < Material->getNumTechniques(); ++TechniqueIndex)
  {
    const auto * Technique = Material->getTechnique(TechniqueIndex);
    if (CMaterialDocument::IsShaderGenerated(*Technique))
      continue;

    ImGui::PushID(static_cast<int>(TechniqueIndex));
    DrawTechnique(_Document, *Technique, TMaterialNode{_MaterialIndex, static_cast<int>(TechniqueIndex)});
    ImGui::PopID();
  }

  ImGui::TreePop();
}

void CMaterialOutlineWindow::DrawTechnique(CMaterialDocument & _Document, const Ogre::Technique & _Technique, const TMaterialNode & _Node)
{
  const auto Label = MakeLabel("Technique", static_cast<std::size_t>(_Node.TechniqueIndex), _Technique.getName());
  if (!DrawNode(_Document, _Node, Label, BranchFlags))
    return;

  for (std::size_t PassIndex = 0; PassIndex < _Technique.getNumPasses(); ++PassIndex)
  {
    ImGui::PushID(static_cast<int>(PassIndex));
    DrawPass(_Document, *_Technique.getPass(PassIndex), TMaterialNode{_Node.MaterialIndex, _Node.TechniqueIndex, static_cast<int>(PassIndex)});
    ImGui::PopID();
  }

  ImGui::TreePop();
}

void CMaterialOutlineWindow::DrawPass(CMaterialDocument & _Document, const Ogre::Pass & _Pass, const TMaterialNode & _Node)
{
  const auto Label            = MakeLabel("Pass", static_cast<std::size_t>(_Node.PassIndex), _Pass.getName());
  const auto TextureUnitCount = _Pass.getNumTextureUnitStates();

  if (TextureUnitCount == 0)
  {
    DrawNode(_Document, _Node, Label, LeafFlags);
    return;
  }

  if (!DrawNode(_Document, _Node, Label, BranchFlags))
    return;

  for (std::size_t TextureUnitIndex = 0; TextureUnitIndex < TextureUnitCount; ++TextureUnitIndex)
  {
    ImGui::PushID(static_cast<int>(TextureUnitIndex));
    DrawTextureUnit(_Document, *_Pass.getTextureUnitState(TextureUnitIndex), TMaterialNode{_Node.MaterialIndex, _Node.TechniqueIndex, _Node.PassIndex, static_cast<int>(TextureUnitIndex)});
    ImGui::PopID();
  }

  ImGui::TreePop();
}

void CMaterialOutlineWindow::DrawTextureUnit(CMaterialDocument & _Document, const Ogre::TextureUnitState & _TextureUnit, const TMaterialNode & _Node)
{
  const auto Label = MakeLabel("Texture", static_cast<std::size_t>(_Node.TextureUnitIndex), _TextureUnit.getTextureName());
  DrawNode(_Document, _Node, Label, LeafFlags);
}

} // namespace editor
