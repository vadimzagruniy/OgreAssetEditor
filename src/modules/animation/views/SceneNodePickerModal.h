#pragma once

#include "ui/ModalWindow.h"

#include <cstddef>
#include <memory>
#include <string>

namespace editor
{

class CAnimationEditor;
class CSceneGraph;

class CSceneNodePickerModal final:
  public ui::CModalWindow
{
public: // Constructor

  explicit CSceneNodePickerModal(std::weak_ptr<CAnimationEditor> _Editor);

protected: // CModalWindow

  void OnContentGui() override;

  void OnCancel() override;

private: // Implementation

  void DrawNode(const CSceneGraph & _Graph, const std::string & _NodeName);

private: // Events

  void OnSelectClicked();

  void OnCancelClicked();

private: // Attributes

  std::weak_ptr<CAnimationEditor> m_Editor;
  std::string                     m_NodeName;
  std::size_t                     m_SelectButtonIndex;
};

} // namespace editor
