#pragma once

#include "editor/inspector/PropertyValue.h"

#include <string>

namespace editor
{

struct TPropertySchema;

class IPropertySource
{
public: // Destructor

  virtual ~IPropertySource();

public: // Interface

  virtual std::string GetTitle() const = 0;

  virtual const TPropertySchema & GetSchema() const = 0;

  virtual TPropertyValue GetValue(const std::string & _Identifier) const = 0;

  virtual void Preview(const std::string & _Identifier, const TPropertyValue & _Value) = 0;

  virtual void Commit() = 0;
};

} // namespace editor
