#include "LightNodeBinding.h"
#include "SceneBindingContext.h"

#include <OgreLight.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>

#include <algorithm>

namespace editor
{

namespace
{

Ogre::Light::LightTypes ToOgreLightType(ELightType _LightType)
{
  switch (_LightType)
  {
    case ELightType::Point:       return Ogre::Light::LT_POINT;
    case ELightType::Directional: return Ogre::Light::LT_DIRECTIONAL;
    case ELightType::Spot:        return Ogre::Light::LT_SPOTLIGHT;
  }

  return Ogre::Light::LT_POINT;
}

} // namespace

//
// Constructor/Destructor
//

CLightNodeBinding::CLightNodeBinding(const TSceneBindingContext & _Context):
  m_SceneManager(_Context.SceneManager),
  m_Node(_Context.Node),
  m_Light(_Context.SceneManager->createLight())
{
  m_Node->attachObject(m_Light);
}

CLightNodeBinding::~CLightNodeBinding()
{
  m_Node->detachObject(m_Light);
  m_SceneManager->destroyLight(m_Light);
}

//
// CSceneNodeBinding
//

bool CLightNodeBinding::OnApply(const CLightNode & _Node, std::string & /*_Error*/)
{
  m_Light->setType(ToOgreLightType(_Node.GetLightType()));
  m_Light->setDiffuseColour(_Node.GetColour());
  m_Light->setSpecularColour(_Node.GetColour());
  m_Light->setPowerScale(_Node.GetIntensity());

  const float Range = std::max(_Node.GetRange(), 0.001f);
  m_Light->setAttenuation(Range, 1.0f, 4.5f / Range, 75.0f / (Range * Range));

  const float OuterAngle = _Node.GetSpotOuterAngle();
  const float InnerAngle = std::min(_Node.GetSpotInnerAngle(), OuterAngle);
  m_Light->setSpotlightRange(Ogre::Degree(InnerAngle), Ogre::Degree(OuterAngle));

  return true;
}

} // namespace editor
