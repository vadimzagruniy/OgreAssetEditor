#pragma once

#include "SceneJson.h"

#include <string>

namespace editor
{

class CSceneInstance;
struct TSceneNodeProperties;

bool ReadScene(const std::string & _Text, CSceneInstance & _Scene, std::string & _Error);

std::string WriteScene(const CSceneInstance & _Scene);

TSceneJson PropertiesToJson(const TSceneNodeProperties & _Properties);

bool PropertiesFromJson(const TSceneJson & _Json, TSceneNodeProperties & _Properties, std::string & _Error);

} // namespace editor
