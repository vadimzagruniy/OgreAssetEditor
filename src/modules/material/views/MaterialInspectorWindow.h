#pragma once

#include "ui/DockedWindow.h"

#include <memory>
#include <string>
#include <vector>

namespace editor
{

namespace ui
{
struct TToolBoxDesc;
} // namespace ui

class CAssetManager;
class CMaterialEditingSession;
struct TPassState;
struct TTextureUnitState;

class CMaterialInspectorWindow final:
  public ui::CDockedWindow
{
public: // Constructor

  CMaterialInspectorWindow(const ui::TToolBoxDesc & _Desc, std::shared_ptr<CMaterialEditingSession> _Session, std::shared_ptr<CAssetManager> _AssetManager);

private: // CWindow

  void OnGui() override;

private: // Implementation

  static bool DrawPassState(TPassState & _State);

  bool DrawTextureUnitState(TTextureUnitState & _State) const;

  std::vector<std::string> CollectTextureNames() const;

private: // Attributes

  std::shared_ptr<CMaterialEditingSession> m_Session;
  std::shared_ptr<CAssetManager>           m_AssetManager;
};

} // namespace editor
