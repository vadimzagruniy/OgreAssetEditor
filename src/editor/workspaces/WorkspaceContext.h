#pragma once

namespace editor
{

class CAssetInstance;
class CEditorWorkspace;

class IWorkspaceContext
{
public: // Destructor

  virtual ~IWorkspaceContext();

public: // Interface

  virtual CEditorWorkspace * GetWorkspace() const noexcept = 0;

  virtual CAssetInstance * GetAssetInstance() const noexcept = 0;
};

} // namespace editor
