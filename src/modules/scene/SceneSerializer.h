#pragma once

#include <string>
#include <vector>

namespace editor
{

class CSceneGraph;
struct TAssetReference;
struct TSceneAnimation;

bool ReadScene(const std::string & _Text, CSceneGraph & _Scene, std::vector<TSceneAnimation> & _Animations, std::string & _Error);

std::string WriteScene(const CSceneGraph & _Scene, const std::vector<TSceneAnimation> & _Animations);

bool CollectSceneAssetReferences(const std::string & _Text, std::vector<TAssetReference> & _AssetRefs, std::string & _Error);

bool ReplaceSceneAssetReferences(const std::string & _Text, const TAssetReference & _OldRef, const TAssetReference & _NewRef, std::string & _NewText, bool & _IsChanged, std::string & _Error);

} // namespace editor
