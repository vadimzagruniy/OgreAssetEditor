#pragma once

#include <cstddef>
#include <filesystem>
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

  void SetActive(std::size_t _Index);

  void Close(std::size_t _Index);

  void CloseActive();

  void CloseAll();

  void CloseAssets(const std::filesystem::path & _Path);

  void RelocateAssets(const std::filesystem::path & _OldPath, const std::filesystem::path & _NewPath, const std::filesystem::path & _AssetsRoot);

  bool HasActive() const noexcept;

  std::size_t GetActiveIndex() const noexcept;

  CEditorWorkspace * GetActive() const noexcept;

  const std::vector<std::unique_ptr<CEditorWorkspace>> & GetWorkspaces() const noexcept;

private: // Implementation

  void RebuildPathIndex();

private: // Attributes

  std::shared_ptr<CAssetTypeRegistry>            m_AssetTypes;
  std::shared_ptr<core::CLog>                    m_Log;
  std::vector<std::unique_ptr<CEditorWorkspace>> m_Workspaces;
  std::unordered_map<std::string, std::size_t>   m_IndexByPath;
  std::size_t                                    m_ActiveIndex;
  bool                                           m_HasActive;
};

} // namespace editor
