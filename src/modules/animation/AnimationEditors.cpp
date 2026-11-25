#include "AnimationEditors.h"
#include "AnimationEditor.h"
#include "editor/workspaces/EditorWorkspace.h"

#include <utility>

namespace editor
{

std::vector<std::shared_ptr<CAnimationEditor>> FindAnimationEditors(const CEditorWorkspace & _Workspace)
{
  std::vector<std::shared_ptr<CAnimationEditor>> Editors;

  for (const auto & Attached : _Workspace.GetAttachedEditors())
  {
    if (auto Editor = std::dynamic_pointer_cast<CAnimationEditor>(Attached))
      Editors.push_back(std::move(Editor));
  }

  return Editors;
}

} // namespace editor
