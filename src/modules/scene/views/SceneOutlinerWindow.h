#pragma once

#include "ui/DockedWindow.h"

#include <array>
#include <memory>
#include <string>

namespace editor
{

namespace ui
{
struct TToolBoxDesc;
} // namespace ui

class CSceneDocument;
class ISceneEditorPresenter;

class CSceneOutlinerWindow final:
  public ui::CDockedWindow
{
public: // Constructor

  CSceneOutlinerWindow(const ui::TToolBoxDesc & _Desc, std::shared_ptr<ISceneEditorPresenter> _Presenter);

private: // CWindow

  void OnGui() override;

private: // Implementation

  void DrawNode(const CSceneDocument & _Document, const std::string & _Name);

  void DrawRenameField(const CSceneDocument & _Document);

  void DrawNodeContextMenu(const std::string & _Name);

  void DrawEmptySpace(const CSceneDocument & _Document);

  void AcceptNodeDrop(const CSceneDocument & _Document, const std::string & _NewParentName) const;

  void HandleShortcuts(const CSceneDocument & _Document);

  void BeginRename(const std::string & _Name);

private: // Attributes

  std::shared_ptr<ISceneEditorPresenter> m_Presenter;
  std::string                            m_RenamingNode;
  std::array<char, 256>                  m_RenameBuffer;
  std::string                            m_RenameError;
  bool                                   m_IsRenameFocusPending;
  std::string                            m_LastSelection;
  bool                                   m_IsRevealPending;
};

} // namespace editor
