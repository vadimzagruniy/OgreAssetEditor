#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace editor
{

namespace core
{
class CLog;
} // namespace core

struct TAssetRecord;
class CAssetTypeRegistry;
class CEditorWorkspace;

class CWorkspaceManager
{
public: // Constructor/Destructor

  CWorkspaceManager(std::shared_ptr<CAssetTypeRegistry> _AssetTypes, std::shared_ptr<core::CLog> _Log);

  ~CWorkspaceManager();

public: // Interface

  bool OpenAsset(const TAssetRecord & _Asset);

  bool AddWorkspace(std::unique_ptr<CEditorWorkspace> _Workspace);

  void Draw();

  void Close(std::size_t _Index);

  void CloseActive();

  void CloseAll();

  CEditorWorkspace * GetActive() const noexcept;

  const std::vector<std::unique_ptr<CEditorWorkspace>> & GetWorkspaces() const noexcept;

private: // Implementation

  void SetActive(std::size_t _Index);

  void RebuildPathIndex();

private: // Attributes

  std::shared_ptr<CAssetTypeRegistry>            m_AssetTypes;
  std::shared_ptr<core::CLog>                    m_Log;
  std::vector<std::unique_ptr<CEditorWorkspace>> m_Workspaces;
  std::unordered_map<std::string, std::size_t>   m_IndexByPath;
  std::size_t                                    m_ActiveIndex = 0;
  bool                                           m_HasActive   = false;
};

} // namespace editor
