#pragma once

#include "ViewportNavigationMode.h"

namespace editor
{

class CViewportCamera;
struct TViewportInput;

class CViewportNavigation
{
public: // Constructor

  CViewportNavigation();

public: // Interface

  void Update(const TViewportInput & _Input, CViewportCamera & _Camera);

  EViewportNavigationMode GetMode() const noexcept;

private: // Implementation

  static EViewportNavigationMode SelectMode(const TViewportInput & _Input);

  bool IsModeHeld(const TViewportInput & _Input) const;

  void ApplyMode(const TViewportInput & _Input, CViewportCamera & _Camera) const;

private: // Attributes

  EViewportNavigationMode m_Mode;
};

} // namespace editor
