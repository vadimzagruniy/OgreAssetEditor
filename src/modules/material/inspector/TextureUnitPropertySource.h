#pragma once

#include "MaterialPropertySource.h"

#include <memory>
#include <optional>
#include <string>

namespace editor
{

struct TTextureUnitState;

class CTextureUnitPropertySource final:
  public CMaterialPropertySource
{
public: // Constructor

  explicit CTextureUnitPropertySource(std::shared_ptr<IMaterialEditorPresenter> _Presenter);

public: // IPropertySource

  const TPropertySchema & GetSchema() const override;

  TPropertyValue GetValue(const std::string & _Identifier) const override;

  void Preview(const std::string & _Identifier, const TPropertyValue & _Value) override;

private: // Implementation

  std::optional<TTextureUnitState> GetState() const;
};

} // namespace editor
