#pragma once

#include "ui/DockedWindow.h"

#include <OgrePrerequisites.h>

#include <memory>
#include <string>

namespace editor
{

namespace ui
{
struct TToolBoxDesc;
} // namespace ui

class CMaterialDocument;
class IMaterialEditorPresenter;
struct TMaterialNode;

class CMaterialOutlineWindow final:
  public ui::CDockedWindow
{
public: // Constructor

  CMaterialOutlineWindow(const ui::TToolBoxDesc & _Desc, std::shared_ptr<IMaterialEditorPresenter> _Presenter);

private: // CWindow

  void OnGui() override;

private: // Implementation

  void DrawMaterial(const CMaterialDocument & _Document, int _MaterialIndex) const;

  void DrawTechnique(const CMaterialDocument & _Document, const Ogre::Technique & _Technique, const TMaterialNode & _Node) const;

  void DrawPass(const CMaterialDocument & _Document, const Ogre::Pass & _Pass, const TMaterialNode & _Node) const;

  void DrawTextureUnit(const CMaterialDocument & _Document, const Ogre::TextureUnitState & _TextureUnit, const TMaterialNode & _Node) const;

  bool DrawNode(const CMaterialDocument & _Document, const TMaterialNode & _Node, const std::string & _Label, int _Flags) const;

private: // Attributes

  std::shared_ptr<IMaterialEditorPresenter> m_Presenter;
};

} // namespace editor
