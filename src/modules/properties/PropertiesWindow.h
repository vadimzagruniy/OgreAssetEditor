#pragma once

#include "controls/PropertyControlContext.h"
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

class CAssetInstance;
class IPropertyControl;
class IPropertySource;
struct TPropertyDesc;

class CPropertiesWindow final:
  public ui::CDockedWindow
{
public: // Constructor/Destructor

  CPropertiesWindow(const ui::TToolBoxDesc & _Desc, TPropertyControlContext _Context);

  ~CPropertiesWindow() override;

private: // CWindow

  void OnGui() override;

private: // Implementation

  void DrawSource(IPropertySource & _Source);

  void DrawAssetInfo(const CAssetInstance & _Instance) const;

  std::vector<std::unique_ptr<IPropertyControl>> CreateControls(const std::vector<TPropertyDesc> & _Descs) const;

  std::unique_ptr<IPropertyControl> CreateControl(const TPropertyDesc & _Desc) const;

private: // Attributes

  TPropertyControlContext                        m_Context;
  std::vector<std::unique_ptr<IPropertyControl>> m_Controls;
  std::string                                    m_SchemaIdentifier;
};

} // namespace editor
