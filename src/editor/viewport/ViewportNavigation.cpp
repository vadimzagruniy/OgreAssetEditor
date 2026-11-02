#include "ViewportNavigation.h"
#include "ViewportCamera.h"
#include "ViewportInput.h"

#include <OgreVector.h>

namespace editor
{

namespace
{

constexpr float DragDollySpeed = 0.02f;

} // namespace

//
// Constructor
//

CViewportNavigation::CViewportNavigation():
  m_Mode(EViewportNavigationMode::None)
{
  // Empty
}

//
// Interface
//

void CViewportNavigation::Update(const TViewportInput & _Input, CViewportCamera & _Camera)
{
  if (m_Mode != EViewportNavigationMode::None && !IsModeHeld(_Input))
    m_Mode = EViewportNavigationMode::None;

  if (m_Mode == EViewportNavigationMode::None && _Input.IsCaptured)
    m_Mode = SelectMode(_Input);

  if (m_Mode != EViewportNavigationMode::None)
  {
    ApplyMode(_Input, _Camera);
    return;
  }

  if (_Input.IsHovered && _Input.Wheel != 0.0f)
    _Camera.Dolly(_Input.Wheel);
}

EViewportNavigationMode CViewportNavigation::GetMode() const noexcept
{
  return m_Mode;
}

//
// Implementation
//

EViewportNavigationMode CViewportNavigation::SelectMode(const TViewportInput & _Input)
{
  if (_Input.IsRightDown)
    return _Input.IsAltDown ? EViewportNavigationMode::Dolly : EViewportNavigationMode::Orbit;

  if (_Input.IsMiddleDown)
    return EViewportNavigationMode::Pan;

  // Plain left drag stays free for selection and gizmos.
  if (_Input.IsLeftDown && _Input.IsAltDown)
    return EViewportNavigationMode::Orbit;

  return EViewportNavigationMode::None;
}

bool CViewportNavigation::IsModeHeld(const TViewportInput & _Input) const
{
  if (!_Input.IsCaptured)
    return false;

  switch (m_Mode)
  {
    case EViewportNavigationMode::Orbit: return _Input.IsLeftDown || _Input.IsRightDown;
    case EViewportNavigationMode::Pan:   return _Input.IsMiddleDown;
    case EViewportNavigationMode::Dolly: return _Input.IsRightDown;
    case EViewportNavigationMode::None:  break;
  }

  return false;
}

void CViewportNavigation::ApplyMode(const TViewportInput & _Input, CViewportCamera & _Camera) const
{
  switch (m_Mode)
  {
    case EViewportNavigationMode::Orbit:
      _Camera.Orbit(_Input.MouseDeltaX, _Input.MouseDeltaY);
      _Camera.Move(Ogre::Vector3(_Input.MoveRight, _Input.MoveUp, -_Input.MoveForward), _Input.DeltaTime, _Input.IsShiftDown);
      break;

    case EViewportNavigationMode::Pan:
      _Camera.Pan(_Input.MouseDeltaX, _Input.MouseDeltaY);
      break;

    case EViewportNavigationMode::Dolly:
      _Camera.Dolly((_Input.MouseDeltaX - _Input.MouseDeltaY) * DragDollySpeed);
      break;

    case EViewportNavigationMode::None:
      break;
  }
}

} // namespace editor
