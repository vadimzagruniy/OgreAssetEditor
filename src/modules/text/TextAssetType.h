#pragma once

#include "editor/assets/AssetType.h"

#include <memory>

namespace editor
{

namespace core
{
class CLog;
} // namespace core

class CTextAssetType final:
  public IAssetType
{
public: // Constructor

  explicit CTextAssetType(std::shared_ptr<core::CLog> _Log);

public: // IAssetType

  const TAssetTypeInfo & GetInfo() const noexcept override;

  EAssetOpenResult OpenAsset(const TAssetRecord & _Asset, const std::string & _ResourceGroup, std::unique_ptr<CEditorWorkspace> & _Workspace, std::string & _Error) const override;

  void GetContextActions(const TAssetRecord & _Asset, std::vector<TAssetMenuEntry> & _Entries) const override;

private: // Attributes

  std::shared_ptr<core::CLog> m_Log;
};

} // namespace editor
