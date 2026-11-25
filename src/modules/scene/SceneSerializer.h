#pragma once

#include <string>
#include <vector>

namespace editor
{

class CSceneGraph;
struct TSceneAnimation;

bool ReadScene(const std::string & _Text, CSceneGraph & _Scene, std::vector<TSceneAnimation> & _Animations, std::string & _Error);

std::string WriteScene(const CSceneGraph & _Scene, const std::vector<TSceneAnimation> & _Animations);

} // namespace editor
