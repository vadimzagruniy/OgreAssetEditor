#include "TextAssetType.h"
#include "TextInstance.h"
#include "TextWorkspace.h"
#include "editor/assets/AssetRecord.h"
#include "editor/assets/AssetTypeInfo.h"

#include <fstream>
#include <iterator>

namespace editor
{

//
// IAssetType
//

const TAssetTypeInfo & CTextAssetType::GetInfo() const noexcept
{
  static const TAssetTypeInfo Info{"text", "Text", {".txt", ".md", ".json", ".xml", ".ini", ".cfg", ".log", ".nut", ".lua", ".h", ".cpp", ".glsl", ".compositor", ".program"}, true};
  return Info;
}

std::unique_ptr<CEditorWorkspace> CTextAssetType::OpenAsset(const TAssetRecord & _Asset, const std::string & /*_ResourceGroup*/, std::string & _Error) const
{
  if (_Asset.SizeBytes > CTextInstance::MaxContentBytes)
  {
    _Error = "Text file exceeds the 16 MiB editor limit.";
    return {};
  }

  std::ifstream File(_Asset.AbsolutePath, std::ios::binary);
  if (!File)
  {
    _Error = "Could not open text file.";
    return {};
  }

  std::string Content((std::istreambuf_iterator<char>(File)), {});
  if (Content.size() > CTextInstance::MaxContentBytes || Content.find('\0') != std::string::npos)
  {
    _Error = "File is too large or contains binary data.";
    return {};
  }

  return std::make_unique<CTextWorkspace>(std::make_unique<CTextInstance>(_Asset, std::move(Content)));
}

} // namespace editor
