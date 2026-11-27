#include "TextAssetType.h"
#include "TextDocument.h"
#include "TextWorkspace.h"
#include "core/Log.h"
#include "editor/assets/AssetMenuEntry.h"
#include "editor/assets/AssetOpenResult.h"
#include "editor/assets/AssetRecord.h"
#include "editor/assets/AssetTypeInfo.h"
#include "platform/Shell.h"

#include <fstream>
#include <iterator>
#include <utility>

namespace editor
{

//
// Constructor
//

CTextAssetType::CTextAssetType(std::shared_ptr<core::CLog> _Log):
  m_Log(std::move(_Log))
{
  // Empty
}

//
// IAssetType
//

const TAssetTypeInfo & CTextAssetType::GetInfo() const noexcept
{
  static const TAssetTypeInfo Info{"text", "Text", {".txt", ".md", ".json", ".xml", ".ini", ".cfg", ".log", ".nut", ".lua", ".h", ".cpp", ".glsl", ".compositor", ".program"}, true};
  return Info;
}

EAssetOpenResult CTextAssetType::OpenAsset(const TAssetRecord & _Asset, const std::string & /*_ResourceGroup*/, std::unique_ptr<CEditorWorkspace> & _Workspace, std::string & _Error) const
{
  if (_Asset.SizeBytes > CTextDocument::MaxContentBytes)
  {
    _Error = "Text file exceeds the 16 MiB editor limit.";
    return EAssetOpenResult::Failed;
  }

  std::ifstream File(_Asset.AbsolutePath, std::ios::binary);
  if (!File)
  {
    _Error = "Could not open text file.";
    return EAssetOpenResult::Failed;
  }

  std::string Content((std::istreambuf_iterator<char>(File)), {});
  if (Content.size() > CTextDocument::MaxContentBytes || Content.find('\0') != std::string::npos)
  {
    _Error = "File is too large or contains binary data.";
    return EAssetOpenResult::Failed;
  }

  _Workspace = std::make_unique<CTextWorkspace>(std::make_shared<CTextDocument>(_Asset, std::move(Content)));

  return EAssetOpenResult::OpenedInWorkspace;
}

void CTextAssetType::GetContextActions(const TAssetRecord & _Asset, std::vector<TAssetMenuEntry> & _Entries) const
{
  _Entries.push_back({"Open in External Editor", "", EAssetMenuGroup::Asset, true, [Log = m_Log, Path = _Asset.AbsolutePath]
  {
    std::string Error;
    if (!platform::OpenWithDefaultApp(Path, Error))
      Log->Write(core::ELogLevel::Error, Error);
  }});
}

} // namespace editor
