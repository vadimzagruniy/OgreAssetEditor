#pragma once

#include "ui/layout/Layout.h"

#include <OgrePrerequisites.h>

#include <cstdint>
#include <functional>
#include <memory>

struct ImVec2;

namespace editor
{

namespace ui
{
class CButton;
class CCheckBox;
} // namespace ui

class CRenderScene;
class CViewportCamera;
class CViewportGrid;
class CViewportNavigation;
struct TViewportFrame;
struct TViewportInput;

class CViewport
{
public: // Constructor/Destructor

  explicit CViewport(CRenderScene & _Scene);

  ~CViewport();

  CViewport(const CViewport &) = delete;

  CViewport & operator=(const CViewport &) = delete;

public: // Interface

  void Draw(const char * _Id);

  void OnPick(std::function<void(const Ogre::Ray &)> _Callback);

  void OnOverlay(std::function<bool(const TViewportFrame &)> _Callback);

private: // Implementation

  void SetupToolbar();

  void DrawToolbar(const ImVec2 & _Origin);

  void SyncContent();

  void HandleShortcuts(const TViewportInput & _Input);

  void HandlePick(const TViewportInput & _Input, const ImVec2 & _Origin, const ImVec2 & _Size) const;

  void UpdateClipRange() const;

  void DrawImage(const ImVec2 & _Origin, const ImVec2 & _Size) const;

private: // Attributes

  CRenderScene &                              m_Scene;
  std::unique_ptr<CViewportCamera>            m_Camera;
  std::unique_ptr<CViewportNavigation>        m_Navigation;
  std::unique_ptr<CViewportGrid>              m_Grid;
  ui::CLayout                                 m_Toolbar;
  ui::CCheckBox &                             m_GridCheckBox;
  ui::CButton &                               m_FrameButton;
  std::uint64_t                               m_ContentRevision;
  std::function<void(const Ogre::Ray &)>      m_PickCallback;
  std::function<bool(const TViewportFrame &)> m_OverlayCallback;
};

} // namespace editor
