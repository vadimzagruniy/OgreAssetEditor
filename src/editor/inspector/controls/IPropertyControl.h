#pragma once

namespace editor
{

class IPropertySource;

class IPropertyControl
{
public: // Destructor

  virtual ~IPropertyControl();

public: // Interface

  virtual void Draw(IPropertySource & _Source) = 0;
};

} // namespace editor
