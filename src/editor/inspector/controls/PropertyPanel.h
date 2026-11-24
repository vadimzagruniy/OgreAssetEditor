#pragma once

#include "PropertyControlContext.h"

#include <memory>
#include <string>
#include <vector>

namespace editor
{

class IPropertyControl;
class IPropertySource;
struct TPropertyDesc;

class CPropertyPanel final
{
public: // Constructor/Destructor

  explicit CPropertyPanel(TPropertyControlContext _Context);

  ~CPropertyPanel();

  CPropertyPanel(const CPropertyPanel &) = delete;

  CPropertyPanel & operator=(const CPropertyPanel &) = delete;

public: // Interface

  void Draw(IPropertySource & _Source);

private: // Implementation

  std::vector<std::unique_ptr<IPropertyControl>> CreateControls(const std::vector<TPropertyDesc> & _Descs) const;

  std::unique_ptr<IPropertyControl> CreateControl(const TPropertyDesc & _Desc) const;

private: // Attributes

  TPropertyControlContext                        m_Context;
  std::vector<std::unique_ptr<IPropertyControl>> m_Controls;
  std::string                                    m_SchemaIdentifier;
};

} // namespace editor
