#include "CreateAssetModal.h"
#include "editor/assets/AssetNode.h"
#include "editor/assets/AssetPath.h"
#include "modules/content_browser/presenter/IContentBrowserPresenter.h"

#include <imgui.h>

#include <algorithm>
#include <utility>

namespace editor
{

namespace
{

const ImVec4 ErrorColour(1.0f, 0.4f, 0.4f, 1.0f);

std::string MakeFolderLabel(const std::filesystem::path & _Folder)
{
  return _Folder.empty() ? "Assets" : "Assets/" + _Folder.generic_string();
}

bool HasSubfolders(const TAssetNode & _Folder)
{
  return std::any_of(_Folder.Children.begin(), _Folder.Children.end(), [](const TAssetNode & _Child)
  {
    return _Child.IsDirectory;
  });
}

} // namespace

//
// Constructor
//

CCreateAssetModal::CCreateAssetModal(std::weak_ptr<IContentBrowserPresenter> _Presenter, TAssetCreationRequest _Request):
  CModalWindow("Create " + _Request.Label, ImVec2(40.0f, 10.0f)),
  m_Presenter(std::move(_Presenter)),
  m_Request(std::move(_Request)),
  m_Folder(m_Request.Folder),
  m_NameInput(ui::CInputText("Name")),
  m_CreateButtonIndex(AddButton("Create", [this] { OnCreateClicked(); })),
  m_IsNameValid(false),
  m_IsFocusRequested(true)
{
  m_NameInput.SetText(m_Request.DefaultName).SetWidth({ui::ESizeMode::Fill});

  AddButton("Cancel", [this]
  {
    OnCancelClicked();
  });
}

//
// CModalWindow
//

void CCreateAssetModal::OnContentGui()
{
  DrawNameInput();
  ImGui::Spacing();

  ImGui::AlignTextToFramePadding();
  ImGui::Text("Folder: %s", MakeFolderLabel(m_Folder).c_str());
  ImGui::SameLine();
  DrawFolderPicker();

  ValidateName();
  DrawNameError();

  if (m_IsNameValid && ImGui::IsKeyPressed(ImGuiKey_Enter, false))
    OnCreateClicked();
}

void CCreateAssetModal::OnCancel()
{
  if (const auto Presenter = m_Presenter.lock())
    Presenter->CancelCreateAsset();
}

//
// Implementation
//

void CCreateAssetModal::DrawFolderPicker()
{
  const auto Presenter = m_Presenter.lock();
  if (!Presenter)
    return;

  const ImGuiStyle & Style = ImGui::GetStyle();

  ImGui::PushStyleColor(ImGuiCol_FrameBg, Style.Colors[ImGuiCol_Button]);
  ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, Style.Colors[ImGuiCol_ButtonHovered]);
  ImGui::PushStyleColor(ImGuiCol_FrameBgActive, Style.Colors[ImGuiCol_ButtonActive]);

  const bool IsOpen = ImGui::BeginCombo("##Folder", "...", ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_WidthFitPreview | ImGuiComboFlags_HeightLarge);

  ImGui::PopStyleColor(3);

  if (!IsOpen)
    return;

  DrawFolder(Presenter->GetRootFolder());

  ImGui::EndCombo();
}

void CCreateAssetModal::DrawFolder(const TAssetNode & _Folder)
{
  const auto & FolderPath = _Folder.Record.RelativePath;

  ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

  if (FolderPath == m_Folder)
    Flags |= ImGuiTreeNodeFlags_Selected;

  if (!HasSubfolders(_Folder))
    Flags |= ImGuiTreeNodeFlags_Leaf;

  if (FolderPath.empty() || IsSameOrInside(m_Folder, FolderPath))
    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);

  const bool IsOpen = ImGui::TreeNodeEx(FolderPath.empty() ? "Assets" : _Folder.Name.c_str(), Flags);

  if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen())
  {
    m_Folder = FolderPath;
    ImGui::CloseCurrentPopup();
  }

  if (!IsOpen)
    return;

  for (const auto & Child : _Folder.Children)
  {
    if (Child.IsDirectory)
      DrawFolder(Child);
  }

  ImGui::TreePop();
}

void CCreateAssetModal::DrawNameInput()
{
  if (m_IsFocusRequested)
  {
    ImGui::SetKeyboardFocusHere();
    m_IsFocusRequested = false;
  }

  m_NameInput.Draw();
}

void CCreateAssetModal::ValidateName()
{
  m_NameError.clear();

  const auto Presenter = m_Presenter.lock();
  m_IsNameValid = Presenter && Presenter->CheckAssetName(m_Folder, m_NameInput.GetText(), m_NameError);

  SetButtonEnabled(m_CreateButtonIndex, m_IsNameValid);
}

void CCreateAssetModal::DrawNameError() const
{
  if (m_NameError.empty())
    return;

  ImGui::Spacing();

  ImGui::PushStyleColor(ImGuiCol_Text, ErrorColour);
  ImGui::TextWrapped("%s", m_NameError.c_str());
  ImGui::PopStyleColor();
}

//
// Events
//

void CCreateAssetModal::OnCreateClicked()
{
  if (const auto Presenter = m_Presenter.lock())
    Presenter->ConfirmCreateAsset(m_Folder, m_NameInput.GetText());

  Close();
}

void CCreateAssetModal::OnCancelClicked()
{
  OnCancel();
  Close();
}

} // namespace editor
