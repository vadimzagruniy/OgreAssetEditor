#include "AssetManager.h"
#include "AssetRecord.h"

#include <OgreResourceGroupManager.h>

#include <algorithm>
#include <system_error>
#include <utility>

namespace editor
{

//
// Constructor/Destructor
//

CAssetManager::CAssetManager()
{
  // Empty
}

CAssetManager::~CAssetManager() = default;

//
// Interface
//

bool CAssetManager::OpenProject(const std::filesystem::path & _Root)
{
  std::error_code ErrorCode;

  m_ProjectRoot = std::filesystem::absolute(_Root, ErrorCode).lexically_normal();
  if (ErrorCode)
  {
    m_LastError = "Cannot resolve project directory: " + ErrorCode.message();
    return false;
  }

  m_AssetsRoot = m_ProjectRoot / "Assets";
  if (!std::filesystem::is_directory(m_AssetsRoot, ErrorCode))
  {
    m_LastError = ErrorCode ? ErrorCode.message() : "Project must contain an Assets directory.";
    return false;
  }

  return Refresh();
}

bool CAssetManager::Refresh()
{
  m_Assets.clear();
  m_LastError.clear();

  std::error_code ErrorCode;
  if (!std::filesystem::is_directory(m_AssetsRoot, ErrorCode))
  {
    m_LastError = "Assets directory does not exist.";
    return false;
  }

  const auto Options = std::filesystem::directory_options::skip_permission_denied;

  for (std::filesystem::recursive_directory_iterator FileIterator(m_AssetsRoot, Options, ErrorCode), EndIterator; FileIterator != EndIterator; FileIterator.increment(ErrorCode))
  {
    if (ErrorCode)
    {
      m_LastError = ErrorCode.message();
      ErrorCode.clear();
      continue;
    }

    std::error_code FileError;
    if (!FileIterator->is_regular_file(FileError) || FileError)
      continue;

    TAssetRecord Record;
    Record.AbsolutePath = FileIterator->path().lexically_normal();
    Record.RelativePath = std::filesystem::relative(Record.AbsolutePath, m_AssetsRoot, FileError);

    if (FileError)
      Record.RelativePath = Record.AbsolutePath.filename();

    Record.SizeBytes = FileIterator->file_size(FileError);

    m_Assets.push_back(std::move(Record));
  }

  std::sort(m_Assets.begin(), m_Assets.end(), [](const TAssetRecord & _Left, const TAssetRecord & _Right)
  {
    return _Left.RelativePath.generic_string() < _Right.RelativePath.generic_string();
  });

  return true;
}

bool CAssetManager::MountResources() const
{
  if (m_AssetsRoot.empty() || !std::filesystem::is_directory(m_AssetsRoot))
    return false;

  auto & ResourceGroups = Ogre::ResourceGroupManager::getSingleton();
  if (!ResourceGroups.resourceGroupExists(ResourceGroup))
    ResourceGroups.createResourceGroup(ResourceGroup);

  ResourceGroups.addResourceLocation(m_AssetsRoot.string(), "FileSystem", ResourceGroup, true, true);

  return true;
}

//
// Accessors
//

const std::filesystem::path & CAssetManager::GetProjectRoot() const noexcept
{
  return m_ProjectRoot;
}

const std::filesystem::path & CAssetManager::GetAssetsRoot() const noexcept
{
  return m_AssetsRoot;
}

const std::vector<TAssetRecord> & CAssetManager::GetAssets() const noexcept
{
  return m_Assets;
}

const std::string & CAssetManager::GetLastError() const noexcept
{
  return m_LastError;
}

} // namespace editor
