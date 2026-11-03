#pragma once

#include <memory>

namespace editor
{

namespace core
{
class CActionQueue;
class CLog;
} // namespace core

class CAssetManager;
class CAssetTypeRegistry;
class CWorkspaceManager;

struct TPropertyControlContext
{
  std::shared_ptr<CAssetManager>      AssetManager;
  std::shared_ptr<CAssetTypeRegistry> AssetTypes;
  std::shared_ptr<CWorkspaceManager>  WorkspaceManager;
  std::shared_ptr<core::CActionQueue> ActionQueue;
  std::shared_ptr<core::CLog>         Log;
};

} // namespace editor
