#pragma once

#include "ui/DockedWindow.h"

#include <memory>

namespace editor
{

namespace ui
{
struct TToolBoxDesc;
} // namespace ui

class ISceneEditorPresenter;
struct TSceneNodeTemplate;

class CSceneObjectsWindow final:
  public ui::CDockedWindow
{
public: // Constructor

  CSceneObjectsWindow(const ui::TToolBoxDesc & _Desc, std::shared_ptr<ISceneEditorPresenter> _Presenter);

private: // CWindow

  void OnGui() override;

private: // Implementation

  void DrawTemplate(const TSceneNodeTemplate & _Template) const;

private: // Attributes

  std::shared_ptr<ISceneEditorPresenter> m_Presenter;
};

} // namespace editor
