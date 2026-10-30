#pragma once

#include "ui/DockedWindow.h"

#include <OgrePrerequisites.h>

#include <memory>

namespace editor
{

namespace ui
{
struct TToolBoxDesc;
} // namespace ui

class CMaterialDocument;
class CMaterialEditingSession;
struct TMaterialNode;

class CMaterialOutlineWindow final:
  public ui::CDockedWindow
{
public: // Constructor

  CMaterialOutlineWindow(const ui::TToolBoxDesc & _Desc, std::shared_ptr<CMaterialEditingSession> _Session);

private: // CWindow

  void OnGui() override;

private: // Implementation

  static void DrawMaterial(CMaterialDocument & _Document, int _MaterialIndex);

  static void DrawTechnique(CMaterialDocument & _Document, const Ogre::Technique & _Technique, const TMaterialNode & _Node);

  static void DrawPass(CMaterialDocument & _Document, const Ogre::Pass & _Pass, const TMaterialNode & _Node);

  static void DrawTextureUnit(CMaterialDocument & _Document, const Ogre::TextureUnitState & _TextureUnit, const TMaterialNode & _Node);

private: // Attributes

  std::shared_ptr<CMaterialEditingSession> m_Session;
};

} // namespace editor
