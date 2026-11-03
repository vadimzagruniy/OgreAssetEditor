#include "SceneDropHandlers.h"
#include "ISceneDropHandler.h"

#include <cassert>
#include <utility>

namespace editor
{

//
// Interface
//

void CSceneDropHandlers::Register(std::shared_ptr<ISceneDropHandler> _Handler)
{
  assert(_Handler);
  m_Handlers.push_back(std::move(_Handler));
}

std::shared_ptr<ISceneDropHandler> CSceneDropHandlers::FindHandler(const TSceneDrop & _Drop, const CSceneDocument & _Document) const
{
  for (const auto & Handler : m_Handlers)
  {
    if (Handler->CanDrop(_Drop, _Document))
      return Handler;
  }

  return nullptr;
}

} // namespace editor
