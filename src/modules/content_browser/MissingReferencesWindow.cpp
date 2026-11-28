#include "MissingReferencesWindow.h"
#include "core/ActionQueue.h"
#include "core/Log.h"
#include "editor/assets/AssetDatabase.h"
#include "editor/assets/AssetDependencyIndex.h"
#include "editor/assets/AssetManager.h"
#include "editor/assets/AssetPayload.h"
#include "editor/assets/AssetRecord.h"
#include "editor/workspaces/WorkspaceManager.h"
#include "ui/ChoiceModal.h"
#include "ui/ChoiceModalDesc.h"
#include "ui/DragPayload.h"
#include "ui/DropPayload.h"
#include "ui/ToolBoxDesc.h"
#include "ui/WindowManager.h"

#include <imgui.h>

#include <filesystem>
#include <utility>

namespace editor
{

namespace
{

std::string GetExtension(const std::string & _Path)
{
  return std::filesystem::path(_Path).extension().string();
}

} // namespace

//
// Constructor
//

CMissingReferencesWindow::CMissingReferencesWindow(const ui::TToolBoxDesc & _Desc, std::shared_ptr<CAssetManager> _AssetManager, std::shared_ptr<const CAssetDatabase> _Assets, std::shared_ptr<CAssetDependencyIndex> _Dependencies, std::shared_ptr<CWorkspaceManager> _WorkspaceManager, std::shared_ptr<core::CActionQueue> _ActionQueue, std::shared_ptr<core::CLog> _Log, ui::CWindowManager & _WindowManager):
  CDockedWindow(_Desc.Title, "MissingReferences", _Desc.Visible),
  m_AssetManager(std::move(_AssetManager)),
  m_Assets(std::move(_Assets)),
  m_Dependencies(std::move(_Dependencies)),
  m_WorkspaceManager(std::move(_WorkspaceManager)),
  m_ActionQueue(std::move(_ActionQueue)),
  m_Log(std::move(_Log)),
  m_WindowManager(_WindowManager),
  m_RefreshButton("Refresh"),
  m_AssetsRevision(0),
  m_IsRefreshRequested(true)
{
  m_RefreshButton.SetTooltip("Read the project files again").OnClick([this]
  {
    m_IsRefreshRequested = true;
  });
}

//
// CWindow
//

void CMissingReferencesWindow::OnGui()
{
  SyncMissingRefs();

  m_RefreshButton.Draw();
  ImGui::SameLine();
  ImGui::TextDisabled("Drop an asset from the Content panel on a row to relink it.");

  if (m_MissingRefs.empty())
  {
    ImGui::TextUnformatted("No missing references.");
    return;
  }

  const auto Flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY;
  if (!ImGui::BeginTable("##MissingReferences", 2, Flags))
    return;

  ImGui::TableSetupScrollFreeze(0, 1);
  ImGui::TableSetupColumn("Missing asset", ImGuiTableColumnFlags_WidthStretch, 2.0f);
  ImGui::TableSetupColumn("Used by", ImGuiTableColumnFlags_WidthStretch, 1.0f);
  ImGui::TableHeadersRow();

  for (const auto & MissingRef : m_MissingRefs)
    DrawMissingRef(MissingRef);

  ImGui::EndTable();
}

//
// Implementation
//

void CMissingReferencesWindow::SyncMissingRefs()
{
  if (!m_IsRefreshRequested && m_AssetsRevision == m_Assets->GetRevision())
    return;

  m_IsRefreshRequested = false;
  m_AssetsRevision     = m_Assets->GetRevision();

  m_Dependencies->Update();
  m_MissingRefs = m_Dependencies->GetMissingRefs();
}

void CMissingReferencesWindow::DrawMissingRef(const TMissingAssetReference & _MissingRef)
{
  const auto & AssetRef = _MissingRef.AssetRef;

  ImGui::TableNextRow();
  ImGui::TableSetColumnIndex(0);

  ImGui::PushID(AssetRef.Id.IsValid() ? AssetRef.Id.ToString().c_str() : AssetRef.PathHint.c_str());

  const std::string Label = AssetRef.PathHint.empty() ? "(no path)" : AssetRef.PathHint;
  ImGui::Selectable(Label.c_str(), false, ImGuiSelectableFlags_SpanAllColumns);

  if (ImGui::IsItemHovered())
    ImGui::SetTooltip("ID: %s", AssetRef.Id.IsValid() ? AssetRef.Id.ToString().c_str() : "none");

  HandleRelinkDrop(_MissingRef);

  ImGui::TableSetColumnIndex(1);
  DrawReferrers(_MissingRef);

  ImGui::PopID();
}

void CMissingReferencesWindow::DrawReferrers(const TMissingAssetReference & _MissingRef) const
{
  const auto & ReferrerIds = _MissingRef.ReferrerIds;

  const std::string Summary = ReferrerIds.size() == 1 ? MakeReferrerPath(ReferrerIds.front()) : std::to_string(ReferrerIds.size()) + " assets";
  ImGui::TextUnformatted(Summary.c_str());

  if (ReferrerIds.size() < 2 || !ImGui::IsItemHovered())
    return;

  ImGui::BeginTooltip();

  for (const auto & ReferrerId : ReferrerIds)
    ImGui::TextUnformatted(MakeReferrerPath(ReferrerId).c_str());

  ImGui::EndTooltip();
}

void CMissingReferencesWindow::HandleRelinkDrop(const TMissingAssetReference & _MissingRef)
{
  if (!GetAssetPayload().Peek() || !ImGui::BeginDragDropTarget())
    return;

  if (const auto Payload = GetAssetPayload().AcceptBeforeDelivery())
  {
    ImGui::SetTooltip("Relink to %s", Payload->Data.c_str());

    if (Payload->IsDelivered)
    {
      m_ActionQueue->Post([this, _MissingRef, ReplacementPath = Payload->Data]
      {
        Relink(_MissingRef, ReplacementPath);
      });
    }
  }

  ImGui::EndDragDropTarget();
}

void CMissingReferencesWindow::Relink(const TMissingAssetReference & _MissingRef, const std::string & _ReplacementPath) const
{
  const auto MissingExtension = GetExtension(_MissingRef.AssetRef.PathHint);
  if (!MissingExtension.empty() && MissingExtension != GetExtension(_ReplacementPath))
  {
    ReportProblem(core::ELogLevel::Warning, "Cannot relink '" + _MissingRef.AssetRef.PathHint + "' to '" + _ReplacementPath + "': the asset types differ.");
    return;
  }

  const auto OpenReferrers = FindOpenReferrers(_MissingRef);
  if (!OpenReferrers.empty())
  {
    ReportProblem(core::ELogLevel::Warning, "Close these assets before relinking '" + _MissingRef.AssetRef.PathHint + "':", OpenReferrers);
    return;
  }

  std::vector<std::filesystem::path> ChangedPaths;
  std::string                        Error;

  if (!m_AssetManager->ReplaceReferences(_MissingRef.AssetRef, _ReplacementPath, ChangedPaths, Error))
    ReportProblem(core::ELogLevel::Error, "Relink failed: " + Error);

  if (!ChangedPaths.empty())
    m_Log->Write(core::ELogLevel::Info, "Relinked '" + _MissingRef.AssetRef.PathHint + "' to '" + _ReplacementPath + "' in " + std::to_string(ChangedPaths.size()) + " asset(s).");
}

std::vector<std::string> CMissingReferencesWindow::FindOpenReferrers(const TMissingAssetReference & _MissingRef) const
{
  std::vector<std::string> OpenReferrers;

  for (const auto & ReferrerId : _MissingRef.ReferrerIds)
  {
    TAssetRecord Referrer;
    if (m_Assets->FindById(ReferrerId, Referrer) && m_WorkspaceManager->IsAssetOpen(Referrer.AbsolutePath))
      OpenReferrers.push_back(Referrer.RelativePath.generic_string());
  }

  return OpenReferrers;
}

std::string CMissingReferencesWindow::MakeReferrerPath(const TAssetId & _ReferrerId) const
{
  TAssetRecord Referrer;

  return m_Assets->FindById(_ReferrerId, Referrer) ? Referrer.RelativePath.generic_string() : _ReferrerId.ToString();
}

void CMissingReferencesWindow::ReportProblem(core::ELogLevel _Level, const std::string & _Message, std::vector<std::string> _Details) const
{
  std::string LogMessage = _Message;
  for (const auto & Detail : _Details)
    LogMessage += " " + Detail;

  m_Log->Write(_Level, LogMessage);

  ui::TChoiceModalDesc Desc;
  Desc.Title   = "Relink";
  Desc.Message = _Message;
  Desc.Details = std::move(_Details);
  Desc.Choices = {{"OK", [](std::string & /*_Error*/) { return true; }}};

  m_WindowManager.OpenModal(std::make_unique<ui::CChoiceModal>(std::move(Desc)));
}

} // namespace editor
