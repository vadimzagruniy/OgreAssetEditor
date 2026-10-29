#include "WorkspaceManager.h"
#include "AssetInstance.h"
#include "EditorWorkspace.h"
#include "core/Log.h"
#include "editor/assets/AssetManager.h"
#include "editor/assets/AssetRecord.h"
#include "editor/assets/AssetType.h"
#include "editor/assets/AssetTypeRegistry.h"

#include <imgui.h>

#include <algorithm>
#include <filesystem>
#include <utility>

namespace editor
{

namespace
{

std::string MakePathKey(const std::filesystem::path & _Path)
{
  return std::filesystem::weakly_canonical(_Path).generic_string();
}

std::string MakePathKey(const CEditorWorkspace & _Workspace)
{
  return MakePathKey(_Workspace.GetInstance()->GetRecord().AbsolutePath);
}

} // namespace

//
// Constructor/Destructor
//

CWorkspaceManager::CWorkspaceManager(std::shared_ptr<CAssetTypeRegistry> _AssetTypes, std::shared_ptr<core::CLog> _Log):
  m_AssetTypes(std::move(_AssetTypes)),
  m_Log(std::move(_Log))
{
  // Empty
}

CWorkspaceManager::~CWorkspaceManager() = default;

//
// Interface
//

bool CWorkspaceManager::OpenAsset(const TAssetRecord & _Asset)
{
  // Already opened asset is just activated, without loading it again.
  if (const auto OpenedIterator = m_IndexByPath.find(MakePathKey(_Asset.AbsolutePath)); OpenedIterator != m_IndexByPath.end())
  {
    SetActive(OpenedIterator->second);
    return true;
  }

  const auto AssetType = m_AssetTypes->FindByExtension(_Asset.RelativePath.extension().string());
  if (!AssetType)
  {
    m_Log->Write(core::ELogLevel::Warning, "No asset module for " + _Asset.RelativePath.string());
    return false;
  }

  std::string Error;
  auto        Workspace = AssetType->OpenAsset(_Asset, CAssetManager::ResourceGroup, Error);

  if (!Workspace)
  {
    m_Log->Write(core::ELogLevel::Error, Error.empty() ? "Asset open failed" : Error);
    return false;
  }

  return AddWorkspace(std::move(Workspace));
}

bool CWorkspaceManager::AddWorkspace(std::unique_ptr<CEditorWorkspace> _Workspace)
{
  if (!_Workspace)
    return false;

  const auto PathKey = MakePathKey(*_Workspace);

  if (const auto OpenedIterator = m_IndexByPath.find(PathKey); OpenedIterator != m_IndexByPath.end())
  {
    SetActive(OpenedIterator->second);
    return false;
  }

  m_IndexByPath.emplace(PathKey, m_Workspaces.size());
  m_Workspaces.push_back(std::move(_Workspace));

  SetActive(m_Workspaces.size() - 1);

  return true;
}

void CWorkspaceManager::Draw()
{
  if (!ImGui::Begin("Resource Editors###AssetEditorHost"))
  {
    ImGui::End();
    return;
  }

  if (ImGui::BeginTabBar("##OpenAssetTabs"))
  {
    for (std::size_t WorkspaceIndex = 0; WorkspaceIndex < m_Workspaces.size(); ++WorkspaceIndex)
    {
      auto &                  Workspace = *m_Workspaces[WorkspaceIndex];
      bool                    IsOpen    = true;
      const ImGuiTabItemFlags TabFlags  = WorkspaceIndex == m_ActiveIndex ? ImGuiTabItemFlags_SetSelected : 0;

      if (ImGui::BeginTabItem(Workspace.GetTitle().c_str(), &IsOpen, TabFlags))
      {
        SetActive(WorkspaceIndex);

        Workspace.OnGui();

        ImGui::EndTabItem();
      }

      if (!IsOpen)
      {
        Close(WorkspaceIndex);
        break;
      }
    }

    ImGui::EndTabBar();
  }

  ImGui::End();
}

void CWorkspaceManager::Close(std::size_t _Index)
{
  if (_Index >= m_Workspaces.size())
    return;

  const bool WasActive = m_HasActive && _Index == m_ActiveIndex;
  if (WasActive)
  {
    m_Workspaces[_Index]->OnDeactivated();
    m_HasActive = false;
  }

  m_Workspaces.erase(m_Workspaces.begin() + static_cast<std::ptrdiff_t>(_Index));
  RebuildPathIndex();

  if (m_Workspaces.empty())
  {
    m_HasActive   = false;
    m_ActiveIndex = 0;
    return;
  }

  if (WasActive)
    SetActive(std::min(_Index, m_Workspaces.size() - 1));
  else
  if (_Index < m_ActiveIndex)
    --m_ActiveIndex;
}

void CWorkspaceManager::CloseActive()
{
  if (m_HasActive)
    Close(m_ActiveIndex);
}

void CWorkspaceManager::CloseAll()
{
  if (m_HasActive)
    m_Workspaces[m_ActiveIndex]->OnDeactivated();

  m_Workspaces.clear();
  m_IndexByPath.clear();

  m_HasActive   = false;
  m_ActiveIndex = 0;
}

CEditorWorkspace * CWorkspaceManager::GetActive() const noexcept
{
  return m_HasActive ? m_Workspaces[m_ActiveIndex].get() : nullptr;
}

const std::vector<std::unique_ptr<CEditorWorkspace>> & CWorkspaceManager::GetWorkspaces() const noexcept
{
  return m_Workspaces;
}

//
// Implementation
//

void CWorkspaceManager::SetActive(std::size_t _Index)
{
  if (m_HasActive && m_ActiveIndex == _Index)
    return;

  if (m_HasActive)
    m_Workspaces[m_ActiveIndex]->OnDeactivated();

  m_ActiveIndex = _Index;
  m_HasActive   = true;

  m_Workspaces[m_ActiveIndex]->OnActivated();
}

void CWorkspaceManager::RebuildPathIndex()
{
  m_IndexByPath.clear();

  for (std::size_t WorkspaceIndex = 0; WorkspaceIndex < m_Workspaces.size(); ++WorkspaceIndex)
    m_IndexByPath.emplace(MakePathKey(*m_Workspaces[WorkspaceIndex]), WorkspaceIndex);
}

} // namespace editor
