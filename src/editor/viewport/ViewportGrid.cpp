#include "ViewportGrid.h"

#include <OgreManualObject.h>
#include <OgreMaterialManager.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreVector.h>

#include <algorithm>
#include <cmath>

namespace editor
{

namespace
{

constexpr const char * MaterialName = "Editor/Grid";

constexpr float CellsAcrossView = 25.0f;
constexpr float MinCellSize     = 0.0001f;
constexpr float FadeFactor      = 6.0f;
constexpr float MinorAlpha      = 0.22f;
constexpr float MajorAlpha      = 0.45f;

} // namespace

//
// Constructor/Destructor
//

CViewportGrid::CViewportGrid(Ogre::SceneManager * _SceneManager):
  m_SceneManager(_SceneManager),
  m_Node(_SceneManager->getRootSceneNode()->createChildSceneNode()),
  m_Plane(nullptr),
  m_IsVisible(true)
{
  CreatePlane();
}

CViewportGrid::~CViewportGrid()
{
  if (m_Plane)
    m_SceneManager->destroyManualObject(m_Plane);

  m_SceneManager->destroySceneNode(m_Node);
}

//
// Interface
//

bool CViewportGrid::IsAvailable() const noexcept
{
  return m_Plane != nullptr;
}

bool CViewportGrid::IsVisible() const noexcept
{
  return m_IsVisible;
}

void CViewportGrid::SetVisible(bool _IsVisible)
{
  m_IsVisible = _IsVisible;
  m_Node->setVisible(m_IsVisible);
}

void CViewportGrid::Update(const Ogre::Vector3 & _CameraPosition, float _ViewDistance)
{
  if (!m_Plane || !m_IsVisible)
    return;

  // Three levels of 1x, 10x and 100x cells; the finest one fades out as the camera moves away,
  // so a level reaches the next one with the same alpha and the switch is not visible.
  const float Level        = std::log10(std::max(_ViewDistance / CellsAcrossView, MinCellSize));
  const float Exponent     = std::floor(Level);
  const float Remaining    = 1.0f - (Level - Exponent);
  const float CellSize     = std::pow(10.0f, Exponent);
  const float FadeDistance = _ViewDistance * FadeFactor;

  auto * Section = m_Plane->getSection(0);
  Section->setCustomParameter(0, Ogre::Vector4(CellSize, FadeDistance, 0.0f, 0.0f));
  Section->setCustomParameter(1, Ogre::Vector4(Remaining * MinorAlpha, MinorAlpha + Remaining * (MajorAlpha - MinorAlpha), MajorAlpha, 0.0f));

  // The plane follows the camera, while the lines stay put because the shader draws them in world space.
  m_Node->setPosition(_CameraPosition.x, 0.0f, _CameraPosition.z);
  m_Node->setScale(FadeDistance, 1.0f, FadeDistance);
}

//
// Implementation
//

void CViewportGrid::CreatePlane()
{
  if (!Ogre::MaterialManager::getSingleton().resourceExists(MaterialName, Ogre::RGN_INTERNAL))
    return;

  m_Plane = m_SceneManager->createManualObject();
  m_Plane->setCastShadows(false);

  m_Plane->begin(MaterialName, Ogre::RenderOperation::OT_TRIANGLE_LIST, Ogre::RGN_INTERNAL);
  m_Plane->position(-1.0f, 0.0f, -1.0f);
  m_Plane->position(1.0f, 0.0f, -1.0f);
  m_Plane->position(1.0f, 0.0f, 1.0f);
  m_Plane->position(-1.0f, 0.0f, 1.0f);
  m_Plane->quad(0, 1, 2, 3);
  m_Plane->end();

  m_Node->attachObject(m_Plane);
}

} // namespace editor
