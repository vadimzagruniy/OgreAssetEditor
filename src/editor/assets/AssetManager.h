#pragma once

#include "AssetNode.h"
#include "core/Signal.h"

#include <filesystem>
#include <functional>
#include <optional>
#include <string>

namespace editor
{

class CAssetManager
{
public: // Constants

  static constexpr const char * ResourceGroup = "OgreResourceEditorProject";

public: // Types

  using TChangedSignal = core::CSignal<>;
  using TRemovedSignal = core::CSignal<const std::filesystem::path &>;
  using TRenamedSignal = core::CSignal<const std::filesystem::path &, const std::filesystem::path &>;

public: // Constructor/Destructor

  CAssetManager();

  ~CAssetManager();

public: // Interface

  bool OpenProject(const std::filesystem::path & _Root);

  bool Refresh();

  bool MountResources();

public: // Operations

  bool CreateFolder(const std::filesystem::path & _ParentFolder, std::filesystem::path & _CreatedPath, std::string & _Error);

  bool Rename(const std::filesystem::path & _Path, const std::string & _NewName, std::filesystem::path & _RenamedPath, std::string & _Error);

  bool Duplicate(const std::filesystem::path & _Path, std::filesystem::path & _CreatedPath, std::string & _Error);

  bool Remove(const std::filesystem::path & _Path, std::string & _Error);

public: // Accessors

  const std::filesystem::path & GetProjectRoot() const noexcept;

  const std::filesystem::path & GetAssetsRoot() const noexcept;

  const TAssetNode & GetRootNode() const noexcept;

  bool HasNode(const std::filesystem::path & _RelativePath) const;

  const TAssetNode & GetNode(const std::filesystem::path & _RelativePath) const;

  const std::string & GetLastError() const noexcept;

public: // Signals

  TChangedSignal & GetChangedSignal() noexcept;

  TRemovedSignal & GetRemovedSignal() noexcept;

  TRenamedSignal & GetRenamedSignal() noexcept;

private: // Implementation

  std::optional<std::reference_wrapper<const TAssetNode>> FindNode(const std::filesystem::path & _RelativePath) const;

  void ScanFolder(TAssetNode & _Folder);

  void RemountResources();

  std::filesystem::path ToAbsolute(const std::filesystem::path & _RelativePath) const;

private: // Attributes

  std::filesystem::path m_ProjectRoot;
  std::filesystem::path m_AssetsRoot;
  TAssetNode            m_RootNode;
  std::string           m_LastError;
  bool                  m_IsResourcesMounted;
  TChangedSignal        m_ChangedSignal;
  TRemovedSignal        m_RemovedSignal;
  TRenamedSignal        m_RenamedSignal;
};

} // namespace editor
