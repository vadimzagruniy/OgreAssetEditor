#include "SceneAssetType.h"
#include "SceneDocument.h"
#include "SceneWorkspace.h"
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

CSceneAssetType::CSceneAssetType(std::shared_ptr<CSceneEditorController> _Controller, std::shared_ptr<const CSceneNodeTypes> _NodeTypes, std::shared_ptr<const CSceneNodeBindings> _NodeBindings, std::shared_ptr<core::CLog> _Log):
  m_Controller(std::move(_Controller)),
  m_NodeTypes(std::move(_NodeTypes)),
  m_NodeBindings(std::move(_NodeBindings)),
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

std::unique_ptr<CEditorWorkspace> CSceneAssetType::OpenAsset(const TAssetRecord & _Asset, const std::string & _ResourceGroup, std::string & _Error) const
{
  std::ifstream File(_Asset.AbsolutePath, std::ios::binary);
  if (!File)
  {
    _Error = "Could not open scene file: " + _Asset.AbsolutePath.string();
    return {};
  }

  const std::string Text((std::istreambuf_iterator<char>(File)), {});

  auto Document = std::make_shared<CSceneDocument>(_Asset, m_NodeTypes);
  if (!Document->Load(Text, _Error))
  {
    _Error = _Asset.RelativePath.generic_string() + ": " + _Error;
    return {};
  }

  return std::make_unique<CSceneWorkspace>(std::move(Document), m_Controller, _ResourceGroup, m_NodeBindings, m_Log);
}

} // namespace editor
