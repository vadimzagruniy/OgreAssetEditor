#pragma once

#include <string>

namespace editor::core
{

class ICommand
{
public: // Destructor

  virtual ~ICommand();

public: // Interface

  virtual std::string GetName() const = 0;

  virtual void Execute() = 0;

  virtual void Undo() = 0;

  virtual bool MergeWith(const ICommand & _Next);
};

} // namespace editor::core
