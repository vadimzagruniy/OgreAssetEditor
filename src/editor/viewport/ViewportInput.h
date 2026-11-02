#pragma once

namespace editor
{

struct TViewportInput
{
  float DeltaTime    = 0.0f;
  float MouseDeltaX  = 0.0f;
  float MouseDeltaY  = 0.0f;
  float Wheel        = 0.0f;
  float MoveRight    = 0.0f;
  float MoveUp       = 0.0f;
  float MoveForward  = 0.0f;
  bool  IsHovered    = false;
  bool  IsCaptured   = false;
  bool  IsLeftDown   = false;
  bool  IsRightDown  = false;
  bool  IsMiddleDown = false;
  bool  IsAltDown    = false;
  bool  IsShiftDown  = false;
};

} // namespace editor
