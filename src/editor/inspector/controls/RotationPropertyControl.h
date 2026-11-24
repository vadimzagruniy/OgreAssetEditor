#pragma once

#include "ValuePropertyControl.h"
#include "ui/elements/DragFloat3.h"

#include <OgreQuaternion.h>

namespace editor
{

struct TPropertyDesc;

class CRotationPropertyControl final:
  public CValuePropertyControl
{
public: // Constructor

  explicit CRotationPropertyControl(const TPropertyDesc & _Desc);

protected: // CValuePropertyControl

  void OnDraw(const TPropertyValue & _Value) override;

private: // Attributes

  ui::CDragFloat3         m_Drag;
  ui::CDragFloat3::TValue m_Angles;
  Ogre::Quaternion        m_Orientation;
};

} // namespace editor
