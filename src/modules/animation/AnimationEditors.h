#pragma once

#include <memory>
#include <vector>

namespace editor
{

class CAnimationEditor;
class CEditorWorkspace;

std::vector<std::shared_ptr<CAnimationEditor>> FindAnimationEditors(const CEditorWorkspace & _Workspace);

} // namespace editor
