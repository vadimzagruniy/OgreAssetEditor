#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace editor
{

struct TAssetRecord;

class CAssetManager
{
public: // Constants

  static constexpr const char * ResourceGroup = "OgreResourceEditorProject";

public: // Constructor/Destructor

  CAssetManager();

  ~CAssetManager();

public: // Interface

  bool OpenProject(const std::filesystem::path & _Root);

  bool Refresh();

  bool MountResources() const;

public: // Accessors

  const std::filesystem::path & GetProjectRoot() const noexcept;

  const std::filesystem::path & GetAssetsRoot() const noexcept;

  const std::vector<TAssetRecord> & GetAssets() const noexcept;

  const std::string & GetLastError() const noexcept;

private: // Attributes

  std::filesystem::path     m_ProjectRoot;
  std::filesystem::path     m_AssetsRoot;
  std::vector<TAssetRecord> m_Assets;
  std::string               m_LastError;
};

} // namespace editor
