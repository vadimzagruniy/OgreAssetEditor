#pragma once

#include <string>

namespace editor
{

class CSceneGraph;

bool ReadScene(const std::string & _Text, CSceneGraph & _Scene, std::string & _Error);

std::string WriteScene(const CSceneGraph & _Scene);

} // namespace editor
