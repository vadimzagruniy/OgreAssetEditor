#pragma once

#include "editor/viewport/gizmo/GizmoDrag.h"
#include "editor/viewport/gizmo/GizmoHandle.h"
#include "editor/viewport/gizmo/GizmoMode.h"
#include "editor/viewport/gizmo/GizmoSpace.h"

#include <OgrePrerequisites.h>

#include <functional>
#include <optional>

struct ImDrawList;

namespace editor
{

class CRenderScene;
struct TGizmoView;
struct TViewportFrame;

class CTransformGizmo
{
public: // Constructor

  explicit CTransformGizmo(const CRenderScene & _Scene);

public: // Interface

  void SetMode(EGizmoMode _Mode);

  EGizmoMode GetMode() const noexcept;

  void SetSpace(EGizmoSpace _Space);

  EGizmoSpace GetSpace() const noexcept;

  bool IsDragging() const noexcept;

  bool Draw(const TViewportFrame & _Frame, const Ogre::Affine3 & _Transform);

  void OnDrag(std::function<void(const Ogre::Affine3 &)> _Callback);

  void OnDragFinished(std::function<void()> _Callback);

  void OnDragCanceled(std::function<void()> _Callback);

private: // Implementation

  std::optional<TGizmoView> BuildView(const TViewportFrame & _Frame, const Ogre::Affine3 & _Transform) const;

  bool CanHover(const TViewportFrame & _Frame) const;

  EGizmoHandle FindHoveredHandle(const TGizmoView & _View) const;

  void BeginDrag(const TGizmoView & _View, const TViewportFrame & _Frame, EGizmoHandle _Handle);

  void UpdateDrag(const std::optional<TGizmoView> & _View, const TViewportFrame & _Frame);

  std::optional<Ogre::Affine3> CalcDragTransform(const TGizmoView & _View, const TViewportFrame & _Frame);

  std::optional<Ogre::Affine3> CalcTranslation(const TViewportFrame & _Frame) const;

  Ogre::Affine3 CalcRotation(const TGizmoView & _View);

  std::optional<Ogre::Affine3> CalcScale(const TGizmoView & _View, const TViewportFrame & _Frame) const;

  Ogre::Ray GetMouseRay(const TViewportFrame & _Frame) const;

  void DrawHandles(ImDrawList & _DrawList, const TGizmoView & _View) const;

  void DrawAxes(ImDrawList & _DrawList, const TGizmoView & _View) const;

  void DrawPlanes(ImDrawList & _DrawList, const TGizmoView & _View) const;

  void DrawRings(ImDrawList & _DrawList, const TGizmoView & _View) const;

  void DrawCenter(ImDrawList & _DrawList, const TGizmoView & _View) const;

  bool IsHighlighted(EGizmoHandle _Handle) const;

private: // Attributes

  const CRenderScene &                       m_Scene;
  EGizmoMode                                 m_Mode;
  EGizmoSpace                                m_Space;
  EGizmoHandle                               m_HoveredHandle;
  std::optional<TGizmoDrag>                  m_Drag;
  std::function<void(const Ogre::Affine3 &)> m_DragCallback;
  std::function<void()>                      m_DragFinishedCallback;
  std::function<void()>                      m_DragCanceledCallback;
};

} // namespace editor
