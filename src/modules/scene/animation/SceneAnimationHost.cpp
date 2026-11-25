#include "SceneAnimationHost.h"
#include "modules/scene/SceneDocument.h"
#include "modules/scene/render/SceneSync.h"

#include <utility>

namespace editor
{

//
// Constructor
//

CSceneAnimationHost::CSceneAnimationHost(std::shared_ptr<const CSceneDocument> _Document, CSceneSync & _Sync):
  m_Document(std::move(_Document)),
  m_Sync(_Sync)
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

Ogre::SceneNode * CSceneAnimationHost::FindSceneNode(const std::string & _Name) const
{
  return m_Sync.FindOgreNode(_Name);
}

void CSceneAnimationHost::RestoreNode(const std::string & _Name)
{
  m_Sync.RestoreTransform(m_Document->GetGraph(), _Name);
}

} // namespace editor
