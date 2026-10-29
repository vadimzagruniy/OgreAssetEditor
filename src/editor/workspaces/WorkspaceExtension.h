#pragma once

namespace editor
{

class IWorkspaceContext;

class IWorkspaceExtension
{
public: // Destructor

  virtual ~IWorkspaceExtension();

public: // Interface

  virtual void Attach(IWorkspaceContext & _Context) = 0;

  virtual void Detach() noexcept = 0;
};

} // namespace editor
