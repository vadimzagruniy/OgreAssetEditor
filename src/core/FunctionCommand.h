#pragma once

#include "Command.h"

#include <functional>
#include <string>

namespace editor::core
{

class CFunctionCommand final:
  public ICommand
{
public: // Constructor

  CFunctionCommand(std::string _Name, std::function<void()> _Execute, std::function<void()> _Undo);

public: // ICommand

  std::string GetName() const override;

  void Execute() override;

  void Undo() override;

private: // Attributes

  std::string           m_Name;
  std::function<void()> m_Execute;
  std::function<void()> m_Undo;
};

} // namespace editor::core
