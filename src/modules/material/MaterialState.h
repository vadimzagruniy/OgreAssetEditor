#pragma once

#include <OgreBlendMode.h>
#include <OgreColourValue.h>
#include <OgreCommon.h>

#include <string>

namespace editor
{

struct TPassState
{
  Ogre::ColourValue      Ambient;
  Ogre::ColourValue      Diffuse;
  Ogre::ColourValue      Specular;
  Ogre::ColourValue      Emissive;
  float                  Shininess           = 0.0f;
  Ogre::SceneBlendFactor SourceBlend         = Ogre::SBF_ONE;
  Ogre::SceneBlendFactor DestBlend           = Ogre::SBF_ZERO;
  bool                   IsDepthWriteEnabled = true;
  Ogre::CullingMode      Culling             = Ogre::CULL_CLOCKWISE;

  bool operator==(const TPassState &) const = default;
};

struct TTextureUnitState
{
  std::string                 TextureName;
  Ogre::TextureAddressingMode AddressU = Ogre::TAM_WRAP;
  Ogre::TextureAddressingMode AddressV = Ogre::TAM_WRAP;
  Ogre::TextureAddressingMode AddressW = Ogre::TAM_WRAP;

  bool operator==(const TTextureUnitState &) const = default;
};

} // namespace editor
