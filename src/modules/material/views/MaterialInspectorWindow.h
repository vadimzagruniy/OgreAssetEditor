#pragma once

#include "ui/DockedWindow.h"
#include "ui/elements/CheckBox.h"
#include "ui/elements/ColorEdit.h"
#include "ui/elements/ComboBox.h"
#include "ui/elements/DragFloat.h"

#include <functional>
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
class IMaterialEditorPresenter;
struct TPassState;
struct TTextureUnitState;

class CMaterialInspectorWindow final:
  public ui::CDockedWindow
{
public: // Constructor

  CMaterialInspectorWindow(const ui::TToolBoxDesc & _Desc, std::shared_ptr<IMaterialEditorPresenter> _Presenter, std::shared_ptr<CAssetManager> _AssetManager);

private: // CWindow

  void OnGui() override;

private: // Implementation

  void SetupPassElements();

  void SetupTextureUnitElements();

  void DrawPassState(const TPassState & _State);

  void DrawTextureUnitState(const TTextureUnitState & _State);

  void EditPassState(const std::function<void(TPassState &)> & _Edit) const;

  void EditTextureUnitState(const std::function<void(TTextureUnitState &)> & _Edit) const;

  void CommitEdit() const;

  std::vector<std::string> CollectTextureNames() const;

private: // Attributes

  std::shared_ptr<IMaterialEditorPresenter> m_Presenter;
  std::shared_ptr<CAssetManager>            m_AssetManager;
  ui::CColorEdit                            m_AmbientEdit;
  ui::CColorEdit                            m_DiffuseEdit;
  ui::CColorEdit                            m_SpecularEdit;
  ui::CColorEdit                            m_EmissiveEdit;
  ui::CDragFloat                            m_ShininessDrag;
  ui::CComboBox                             m_BlendingCombo;
  ui::CCheckBox                             m_DepthWriteCheck;
  ui::CComboBox                             m_CullingCombo;
  ui::CComboBox                             m_ImageCombo;
  ui::CComboBox                             m_AddressingCombo;
};

} // namespace editor
