#pragma once

#include "IPropertyControl.h"

#include <memory>
#include <string>
#include <vector>

namespace editor
{

class CGroupPropertyControl final:
  public IPropertyControl
{
public: // Constructor

  CGroupPropertyControl(std::string _Label, std::vector<std::unique_ptr<IPropertyControl>> _Children);

public: // IPropertyControl

  void Draw(IPropertySource & _Source) override;

private: // Attributes

  std::string                                    m_Label;
  std::vector<std::unique_ptr<IPropertyControl>> m_Children;
};

} // namespace editor
