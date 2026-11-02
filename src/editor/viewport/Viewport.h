#pragma once

#include "ui/layout/Layout.h"

#include <cstdint>
#include <memory>

struct ImVec2;

namespace editor
{

namespace ui
{
class CButton;
class CCheckBox;
} // namespace ui

class CPreviewScene;
class CViewportCamera;
class CViewportGrid;
class CViewportNavigation;
struct TViewportInput;

class CViewport
{
public: // Constructor/Destructor

  explicit CViewport(CPreviewScene & _Scene);

  ~CViewport();

  CViewport(const CViewport &) = delete;

  CViewport & operator=(const CViewport &) = delete;

public: // Interface

  void Draw(const char * _Id);

private: // Implementation

  void SetupToolbar();

  void DrawToolbar(const ImVec2 & _Origin);

  void SyncContent();

  void HandleShortcuts(const TViewportInput & _Input);

  void DrawImage(const ImVec2 & _Origin, const ImVec2 & _Size) const;

private: // Attributes

  CPreviewScene &                      m_Scene;
  std::unique_ptr<CViewportCamera>     m_Camera;
  std::unique_ptr<CViewportNavigation> m_Navigation;
  std::unique_ptr<CViewportGrid>       m_Grid;
  ui::CLayout                          m_Toolbar;
  ui::CCheckBox &                      m_GridCheckBox;
  ui::CButton &                        m_FrameButton;
  std::uint64_t                        m_ContentRevision;
};

} // namespace editor
