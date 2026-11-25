#include "SceneAnimationHost.h"
#include "modules/scene/SceneDocument.h"

#include <utility>

namespace editor
{

//
// Constructor
//

CSceneAnimationHost::CSceneAnimationHost(std::shared_ptr<const CSceneDocument> _Document):
  m_Document(std::move(_Document))
{
  // Empty
}

//
// ISceneAnimationHost
//

const CSceneGraph & CSceneAnimationHost::GetGraph() const
{
  return m_Document->GetGraph();
}

} // namespace editor
