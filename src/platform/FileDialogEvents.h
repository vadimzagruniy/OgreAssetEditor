#pragma once

#include <windows.h>
#include <shobjidl.h>

#include <filesystem>

namespace editor::platform
{

class CFileDialogEvents final:
  public IFileDialogEvents
{
public: // Constructor

  explicit CFileDialogEvents(std::filesystem::path _RootFolder);

public: // IUnknown

  IFACEMETHODIMP QueryInterface(REFIID _InterfaceId, void ** _Object) override;

  IFACEMETHODIMP_(ULONG) AddRef() override;

  IFACEMETHODIMP_(ULONG) Release() override;

public: // IFileDialogEvents

  IFACEMETHODIMP OnFileOk(IFileDialog * _Dialog) override;

  IFACEMETHODIMP OnFolderChanging(IFileDialog * _Dialog, IShellItem * _Folder) override;

  IFACEMETHODIMP OnFolderChange(IFileDialog * _Dialog) override;

  IFACEMETHODIMP OnSelectionChange(IFileDialog * _Dialog) override;

  IFACEMETHODIMP OnShareViolation(IFileDialog * _Dialog, IShellItem * _Item, FDE_SHAREVIOLATION_RESPONSE * _Response) override;

  IFACEMETHODIMP OnTypeChange(IFileDialog * _Dialog) override;

  IFACEMETHODIMP OnOverwrite(IFileDialog * _Dialog, IShellItem * _Item, FDE_OVERWRITE_RESPONSE * _Response) override;

private: // Implementation

  bool IsInsideRoot(IShellItem & _Item) const;

private: // Attributes

  std::filesystem::path m_RootFolder;
  ULONG                 m_ReferenceCount;
};

} // namespace editor::platform
