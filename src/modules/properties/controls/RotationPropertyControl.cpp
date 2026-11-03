#include "RotationPropertyControl.h"
#include "editor/inspector/PropertyDesc.h"

#include <OgreMatrix3.h>

namespace editor
{

namespace
{

ui::CDragFloat3::TValue ToAngles(const Ogre::Quaternion & _Orientation)
{
  Ogre::Matrix3 Rotation;
  _Orientation.ToRotationMatrix(Rotation);

  Ogre::Radian Yaw;
  Ogre::Radian Pitch;
  Ogre::Radian Roll;
  Rotation.ToEulerAnglesYXZ(Yaw, Pitch, Roll);

  return {Pitch.valueDegrees(), Yaw.valueDegrees(), Roll.valueDegrees()};
}

Ogre::Quaternion ToOrientation(const ui::CDragFloat3::TValue & _Angles)
{
  Ogre::Matrix3 Rotation;
  Rotation.FromEulerAnglesYXZ(Ogre::Degree(_Angles[1]), Ogre::Degree(_Angles[0]), Ogre::Degree(_Angles[2]));

  return Ogre::Quaternion(Rotation);
}

} // namespace

//
// Constructor
//

CRotationPropertyControl::CRotationPropertyControl(const TPropertyDesc & _Desc):
  CValuePropertyControl(_Desc.Identifier),
  m_Drag(_Desc.Label),
  m_Angles{0.0f, 0.0f, 0.0f},
  m_Orientation(Ogre::Quaternion::IDENTITY)
{
  const auto & Meta = _Desc.Meta;

  m_Drag.SetRange(Meta.Minimum, Meta.Maximum).SetSpeed(Meta.Speed).SetFormat(Meta.Format).SetEnabled(!Meta.IsReadOnly).OnChanged([this](const ui::CDragFloat3::TValue & _Angles)
  {
    m_Angles      = _Angles;
    m_Orientation = ToOrientation(_Angles);

    SetEditedValue(m_Orientation);
  }).OnEditEnd([this]
  {
    FinishEdit();
  });
}

//
// CValuePropertyControl
//

void CRotationPropertyControl::OnDraw(const TPropertyValue & _Value)
{
  const auto * Orientation = std::get_if<Ogre::Quaternion>(&_Value);
  if (Orientation && *Orientation != m_Orientation)
  {
    m_Orientation = *Orientation;
    m_Angles      = ToAngles(*Orientation);
  }

  m_Drag.SetValue(m_Angles);
  m_Drag.Draw();
}

} // namespace editor
