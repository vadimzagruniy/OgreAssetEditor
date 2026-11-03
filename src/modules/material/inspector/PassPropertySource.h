#pragma once

#include "MaterialPropertySource.h"

#include <memory>
#include <optional>
#include <string>

namespace editor
{

struct TPassState;

class CPassPropertySource final:
  public CMaterialPropertySource
{
public: // Constructor

  explicit CPassPropertySource(std::shared_ptr<IMaterialEditorPresenter> _Presenter);

public: // IPropertySource

  const TPropertySchema & GetSchema() const override;

  TPropertyValue GetValue(const std::string & _Identifier) const override;

  void Preview(const std::string & _Identifier, const TPropertyValue & _Value) override;

private: // Implementation

  std::optional<TPassState> GetState() const;
};

} // namespace editor
