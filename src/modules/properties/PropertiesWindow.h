#pragma once

#include "editor/inspector/controls/PropertyControlContext.h"
#include "ui/DockedWindow.h"

#include <memory>

namespace editor
{

namespace ui
{
struct TToolBoxDesc;
} // namespace ui

class CAssetDocument;
class CPropertyPanel;
class CWorkspaceController;
class CWorkspaceManager;
class IPropertySource;

class CPropertiesWindow final:
  public ui::CDockedWindow
{
public: // Constructor/Destructor

  CPropertiesWindow(const ui::TToolBoxDesc & _Desc, TPropertyControlContext _Context, std::shared_ptr<CWorkspaceController> _WorkspaceController);

  ~CPropertiesWindow() override;

private: // CWindow

  void OnGui() override;

private: // Implementation

  void DrawSource(IPropertySource & _Source);

  void DrawAssetInfo(const CAssetDocument & _Document) const;

private: // Attributes

  std::shared_ptr<CWorkspaceManager>    m_WorkspaceManager;
  std::shared_ptr<CWorkspaceController> m_WorkspaceController;
  std::unique_ptr<CPropertyPanel>       m_Panel;
};

} // namespace editor
