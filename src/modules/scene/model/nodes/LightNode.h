#pragma once

#include "LightType.h"
#include "modules/scene/model/SceneNode.h"

#include <OgreColourValue.h>

#include <string>
#include <string_view>
#include <vector>

namespace editor
{

class CLightNode final:
  public CSceneNode
{
public: // Constants

  static constexpr std::string_view TypeId = "light";

public: // Constructor

  CLightNode();

public: // CSceneNode

  std::string_view GetTypeId() const noexcept override;

  void DescribeProperties(std::vector<TPropertyDesc> & _Properties) const override;

  TPropertyValue GetValue(const std::string & _Identifier) const override;

  bool SetValue(const std::string & _Identifier, const TPropertyValue & _Value) override;

protected: // CSceneNode

  void OnWriteData(TJson & _Data) const override;

  bool OnReadData(const TJson & _Data, std::string & _Error) override;

public: // Interface

  ELightType GetLightType() const noexcept;

  void SetLightType(ELightType _LightType) noexcept;

  const Ogre::ColourValue & GetColour() const noexcept;

  float GetIntensity() const noexcept;

  float GetRange() const noexcept;

  float GetSpotInnerAngle() const noexcept;

  float GetSpotOuterAngle() const noexcept;

private: // Attributes

  ELightType        m_LightType;
  Ogre::ColourValue m_Colour;
  float             m_Intensity;
  float             m_Range;
  float             m_SpotInnerAngle;
  float             m_SpotOuterAngle;
};

} // namespace editor
