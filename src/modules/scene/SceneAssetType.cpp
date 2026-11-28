#include "SceneAssetType.h"
#include "SceneDocument.h"
#include "SceneWorkspace.h"
#include "core/PathUtf8.h"
#include "editor/assets/AssetOpenResult.h"
#include "editor/assets/AssetRecord.h"
#include "editor/assets/AssetTypeInfo.h"

#include <fstream>
#include <iterator>
#include <utility>

namespace editor
{

//
// Constructor
//

CSceneAssetType::CSceneAssetType(std::shared_ptr<CSceneEditorController> _Controller, std::shared_ptr<const CSceneNodeTypes> _NodeTypes, std::shared_ptr<const CSceneNodeBindings> _NodeBindings, std::shared_ptr<const CAssetDatabase> _Assets, std::shared_ptr<core::CLog> _Log):
  m_Controller(std::move(_Controller)),
  m_NodeTypes(std::move(_NodeTypes)),
  m_NodeBindings(std::move(_NodeBindings)),
  m_Assets(std::move(_Assets)),
  m_Log(std::move(_Log))
{
  // Empty
}

//
// IAssetType
//

const TAssetTypeInfo & CSceneAssetType::GetInfo() const noexcept
{
  static const TAssetTypeInfo Info{"scene", "Scene", {".oscene"}, true};
  return Info;
}

EAssetOpenResult CSceneAssetType::OpenAsset(const TAssetRecord & _Asset, const std::string & _ResourceGroup, std::unique_ptr<CEditorWorkspace> & _Workspace, std::string & _Error) const
{
  std::ifstream File(_Asset.AbsolutePath, std::ios::binary);
  if (!File)
  {
    _Error = "Could not open scene file: " + core::PathToUtf8(_Asset.AbsolutePath);
    return EAssetOpenResult::Failed;
  }

  const std::string Text((std::istreambuf_iterator<char>(File)), {});

  auto Document = std::make_shared<CSceneDocument>(_Asset, m_NodeTypes, m_Assets);
  if (!Document->Load(Text, _Error))
  {
    _Error = _Asset.RelativePath.generic_string() + ": " + _Error;
    return EAssetOpenResult::Failed;
  }

  _Workspace = std::make_unique<CSceneWorkspace>(std::move(Document), m_Controller, _ResourceGroup, m_NodeBindings, m_Assets, m_Log);

  return EAssetOpenResult::OpenedInWorkspace;
}

} // namespace editor
