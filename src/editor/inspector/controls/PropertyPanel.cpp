#include "PropertyPanel.h"
#include "AssetPropertyControl.h"
#include "BoolPropertyControl.h"
#include "ColorPropertyControl.h"
#include "EnumPropertyControl.h"
#include "FloatPropertyControl.h"
#include "GroupPropertyControl.h"
#include "RotationPropertyControl.h"
#include "TextPropertyControl.h"
#include "Vector3PropertyControl.h"
#include "editor/inspector/IPropertySource.h"
#include "editor/inspector/PropertyDesc.h"
#include "editor/inspector/PropertySchema.h"

#include <utility>

namespace editor
{

//
// Constructor/Destructor
//

CPropertyPanel::CPropertyPanel(TPropertyControlContext _Context):
  m_Context(std::move(_Context))
{
  // Empty
}

CPropertyPanel::~CPropertyPanel() = default;

//
// Interface
//

void CPropertyPanel::Draw(IPropertySource & _Source)
{
  const auto & Schema = _Source.GetSchema();
  if (Schema.Identifier != m_SchemaIdentifier)
  {
    m_SchemaIdentifier = Schema.Identifier;
    m_Controls         = CreateControls(Schema.Properties);
  }

  for (const auto & Control : m_Controls)
    Control->Draw(_Source);
}

//
// Implementation
//

std::vector<std::unique_ptr<IPropertyControl>> CPropertyPanel::CreateControls(const std::vector<TPropertyDesc> & _Descs) const
{
  std::vector<std::unique_ptr<IPropertyControl>> Controls;
  Controls.reserve(_Descs.size());

  for (const auto & Desc : _Descs)
    Controls.push_back(CreateControl(Desc));

  return Controls;
}

std::unique_ptr<IPropertyControl> CPropertyPanel::CreateControl(const TPropertyDesc & _Desc) const
{
  switch (_Desc.Type)
  {
    case EPropertyType::Group:    return std::make_unique<CGroupPropertyControl>(_Desc.Label, CreateControls(_Desc.Children));
    case EPropertyType::Bool:     return std::make_unique<CBoolPropertyControl>(_Desc);
    case EPropertyType::Float:    return std::make_unique<CFloatPropertyControl>(_Desc);
    case EPropertyType::Enum:     return std::make_unique<CEnumPropertyControl>(_Desc);
    case EPropertyType::Text:     return std::make_unique<CTextPropertyControl>(_Desc);
    case EPropertyType::Asset:    return std::make_unique<CAssetPropertyControl>(_Desc, m_Context);
    case EPropertyType::Vector3:  return std::make_unique<CVector3PropertyControl>(_Desc);
    case EPropertyType::Rotation: return std::make_unique<CRotationPropertyControl>(_Desc);
    case EPropertyType::Color:    return std::make_unique<CColorPropertyControl>(_Desc);
  }

  return std::make_unique<CTextPropertyControl>(_Desc);
}

} // namespace editor
