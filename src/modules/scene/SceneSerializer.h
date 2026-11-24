#pragma once

#include "SceneJson.h"

#include <string>

namespace editor
{

class CSceneGraph;
struct TSceneNodeProperties;

bool ReadScene(const std::string & _Text, CSceneGraph & _Scene, std::string & _Error);

std::string WriteScene(const CSceneGraph & _Scene);

TSceneJson PropertiesToJson(const TSceneNodeProperties & _Properties);

bool PropertiesFromJson(const TSceneJson & _Json, TSceneNodeProperties & _Properties, std::string & _Error);

} // namespace editor
