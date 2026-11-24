#include "AssetPropertyControl.h"
#include "core/ActionQueue.h"
#include "core/Log.h"
#include "editor/assets/AssetManager.h"
#include "editor/assets/AssetNode.h"
#include "editor/assets/AssetPayload.h"
#include "editor/assets/AssetType.h"
#include "editor/assets/AssetTypeInfo.h"
#include "editor/assets/AssetTypeRegistry.h"
#include "editor/inspector/PropertyDesc.h"
#include "editor/workspaces/WorkspaceManager.h"
#include "platform/FileDialog.h"
#include "platform/FileDialogDesc.h"
#include "ui/DragPayload.h"
#include "ui/DropPayload.h"

#include <imgui.h>

#include <algorithm>
#include <utility>

namespace editor
{

namespace
{

const ImVec4 MissingColour(0.90f, 0.35f, 0.35f, 1.0f);

} // namespace

//
// Constructor
//

CAssetPropertyControl::CAssetPropertyControl(const TPropertyDesc & _Desc, TPropertyControlContext _Context):
  CValuePropertyControl(_Desc.Identifier),
  m_Context(std::move(_Context)),
  m_AssetType(_Desc.Meta.AssetType),
  m_PathInput("##Path"),
  m_BrowseButton("..."),
  m_OpenButton("##Open", ImGuiDir_Right),
  m_LabelText(_Desc.Label),
  m_IsReadOnly(_Desc.Meta.IsReadOnly)
{
  SetupElements();
}

//
// CValuePropertyControl
//

void CAssetPropertyControl::OnDraw(const TPropertyValue & _Value)
{
  if (m_BrowsedPath)
  {
    Assign(*m_BrowsedPath);
    m_BrowsedPath.reset();
  }

  SyncAppliedPath(_Value);
  DrawElements();
}

//
// Implementation
//

void CAssetPropertyControl::SetupElements()
{
  m_PathInput.SetHint("None").SetEnabled(!m_IsReadOnly).OnEditEnd([this]
  {
    ApplyTypedPath();
  });

  m_BrowseButton.SetTooltip("Browse the project").SetEnabled(!m_IsReadOnly).OnClick([this]
  {
    m_Context.ActionQueue->Post([this]
    {
      Browse();
    });
  });

  m_OpenButton.SetTooltip("Open in editor").OnClick([this]
  {
    OpenAsset();
  });
}

void CAssetPropertyControl::SyncAppliedPath(const TPropertyValue & _Value)
{
  const auto *      AssetPath = std::get_if<std::string>(&_Value);
  const std::string Current   = AssetPath ? *AssetPath : std::string();

  if (Current == m_AppliedPath)
    return;

  m_AppliedPath = Current;
  m_PathInput.SetText(Current);
}

void CAssetPropertyControl::DrawElements()
{
  const ImGuiStyle & Style      = ImGui::GetStyle();
  const float        ButtonSize = ImGui::GetFrameHeight();
  const float        Spacing    = Style.ItemInnerSpacing.x;
  const float        PathWidth  = std::max(ImGui::CalcItemWidth() - (ButtonSize + Spacing) * 2.0f, 1.0f);
  const bool         IsMissing  = !IsResolved();

  m_PathInput.SetColor(ImGuiCol_Text, IsMissing ? MissingColour : Style.Colors[ImGuiCol_Text]);
  m_PathInput.SetTooltip(IsMissing ? "Asset is not found in the project" : m_AppliedPath);
  m_PathInput.Draw(ImVec2(PathWidth, ButtonSize));

  HandleDrop();

  ImGui::SameLine(0.0f, Spacing);
  m_BrowseButton.Draw(ImVec2(ButtonSize, ButtonSize));

  ImGui::SameLine(0.0f, Spacing);
  m_OpenButton.SetEnabled(!m_AppliedPath.empty() && !IsMissing).Draw();

  ImGui::SameLine(0.0f, Spacing);
  m_LabelText.Draw();
}

void CAssetPropertyControl::HandleDrop()
{
  if (m_IsReadOnly)
    return;

  const auto AssetPath = GetAssetPayload().Peek();
  if (!AssetPath || !IsMatchingType(*AssetPath) || !ImGui::BeginDragDropTarget())
    return;

  if (const auto Payload = GetAssetPayload().AcceptBeforeDelivery())
  {
    ImGui::SetTooltip("Assign %s", std::filesystem::path(Payload->Data).stem().string().c_str());

    if (Payload->IsDelivered)
      Assign(Payload->Data);
  }

  ImGui::EndDragDropTarget();
}

void CAssetPropertyControl::ApplyTypedPath()
{
  const std::string TypedPath = m_PathInput.GetText();
  if (TypedPath == m_AppliedPath)
    return;

  if (TypedPath.empty())
  {
    Assign(std::string());
    return;
  }

  const auto AssetPath = ResolveAssetPath(TypedPath);
  if (!AssetPath)
  {
    Warn("'" + TypedPath + "' is not a " + m_AssetType + " asset of the project.");
    m_PathInput.SetText(m_AppliedPath);
    return;
  }

  Assign(*AssetPath);
}

void CAssetPropertyControl::Browse()
{
  const auto & AssetsRoot = m_Context.AssetManager->GetAssetsRoot();

  platform::TFileDialogDesc Desc;
  Desc.Title         = "Select " + m_AssetType;
  Desc.RootFolder    = AssetsRoot;
  Desc.InitialFolder = IsResolved() && !m_AppliedPath.empty() ? (AssetsRoot / m_AppliedPath).parent_path() : AssetsRoot;
  Desc.FilterName    = m_AssetType;

  if (const auto AssetType = FindAssetType())
  {
    Desc.Title      = "Select " + AssetType->GetInfo().Name;
    Desc.FilterName = AssetType->GetInfo().Name;
    Desc.Extensions = AssetType->GetInfo().Extensions;
  }

  std::filesystem::path SelectedPath;
  std::string           Error;
  if (!platform::OpenFileDialog(Desc, SelectedPath, Error))
  {
    if (!Error.empty())
      Warn(Error);

    return;
  }

  const auto AssetPath = ResolveAssetPath(SelectedPath);
  if (!AssetPath)
  {
    Warn("'" + SelectedPath.string() + "' is not a " + m_AssetType + " asset of the project.");
    return;
  }

  m_BrowsedPath = *AssetPath;
}

void CAssetPropertyControl::OpenAsset() const
{
  m_Context.ActionQueue->Post([AssetManager = m_Context.AssetManager, WorkspaceManager = m_Context.WorkspaceManager, AssetPath = m_AppliedPath]
  {
    if (AssetManager->HasNode(AssetPath))
      WorkspaceManager->OpenAsset(AssetManager->GetNode(AssetPath).Record);
  });
}

void CAssetPropertyControl::Assign(const std::string & _AssetPath)
{
  m_PathInput.SetText(_AssetPath);

  SetEditedValue(_AssetPath);
  FinishEdit();
}

std::optional<std::string> CAssetPropertyControl::ResolveAssetPath(const std::filesystem::path & _Path) const
{
  std::filesystem::path RelativePath = _Path;

  if (_Path.is_absolute())
  {
    const auto InsidePath = platform::FindRelativePath(_Path, m_Context.AssetManager->GetAssetsRoot());
    if (!InsidePath)
      return std::nullopt;

    RelativePath = *InsidePath;
  }

  RelativePath = RelativePath.lexically_normal();
  if (RelativePath.empty() || *RelativePath.begin() == "..")
    return std::nullopt;

  if (!m_Context.AssetManager->HasNode(RelativePath) || !IsMatchingType(RelativePath))
    return std::nullopt;

  return RelativePath.generic_string();
}

bool CAssetPropertyControl::IsMatchingType(const std::filesystem::path & _AssetPath) const
{
  const auto AssetType = m_Context.AssetTypes->FindByExtension(_AssetPath.extension().string());
  return AssetType && AssetType->GetInfo().Identifier == m_AssetType;
}

bool CAssetPropertyControl::IsResolved() const
{
  if (m_AppliedPath.empty())
    return true;

  return m_Context.AssetManager->HasNode(m_AppliedPath) && IsMatchingType(m_AppliedPath);
}

std::shared_ptr<IAssetType> CAssetPropertyControl::FindAssetType() const
{
  const auto & AssetTypes   = m_Context.AssetTypes->GetTypes();
  const auto   TypeIterator = AssetTypes.find(m_AssetType);

  return TypeIterator == AssetTypes.end() ? nullptr : TypeIterator->second;
}

void CAssetPropertyControl::Warn(const std::string & _Message) const
{
  if (m_Context.Log)
    m_Context.Log->Write(core::ELogLevel::Warning, _Message);
}

} // namespace editor
