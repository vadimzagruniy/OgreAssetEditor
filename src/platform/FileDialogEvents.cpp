#include "FileDialogEvents.h"
#include "FileDialog.h"
#include "ShellItem.h"

#include <wrl/client.h>

#include <utility>

namespace editor::platform
{

//
// Constructor
//

CFileDialogEvents::CFileDialogEvents(std::filesystem::path _RootFolder):
  m_RootFolder(std::move(_RootFolder)),
  m_ReferenceCount(1)
{
  // Empty
}

//
// IUnknown
//

IFACEMETHODIMP CFileDialogEvents::QueryInterface(REFIID _InterfaceId, void ** _Object)
{
  if (!_Object)
    return E_POINTER;

  if (_InterfaceId != IID_IUnknown && _InterfaceId != IID_IFileDialogEvents)
  {
    *_Object = nullptr;
    return E_NOINTERFACE;
  }

  *_Object = static_cast<IFileDialogEvents *>(this);
  AddRef();

  return S_OK;
}

IFACEMETHODIMP_(ULONG) CFileDialogEvents::AddRef()
{
  return ++m_ReferenceCount;
}

IFACEMETHODIMP_(ULONG) CFileDialogEvents::Release()
{
  return --m_ReferenceCount;
}

//
// IFileDialogEvents
//

IFACEMETHODIMP CFileDialogEvents::OnFileOk(IFileDialog * _Dialog)
{
  Microsoft::WRL::ComPtr<IShellItem> Result;
  if (FAILED(_Dialog->GetResult(&Result)))
    return S_FALSE;

  return IsInsideRoot(*Result.Get()) ? S_OK : S_FALSE;
}

IFACEMETHODIMP CFileDialogEvents::OnFolderChanging(IFileDialog * /*_Dialog*/, IShellItem * _Folder)
{
  return IsInsideRoot(*_Folder) ? S_OK : E_ACCESSDENIED;
}

IFACEMETHODIMP CFileDialogEvents::OnFolderChange(IFileDialog * /*_Dialog*/)
{
  return S_OK;
}

IFACEMETHODIMP CFileDialogEvents::OnSelectionChange(IFileDialog * /*_Dialog*/)
{
  return S_OK;
}

IFACEMETHODIMP CFileDialogEvents::OnShareViolation(IFileDialog * /*_Dialog*/, IShellItem * /*_Item*/, FDE_SHAREVIOLATION_RESPONSE * /*_Response*/)
{
  return E_NOTIMPL;
}

IFACEMETHODIMP CFileDialogEvents::OnTypeChange(IFileDialog * /*_Dialog*/)
{
  return S_OK;
}

IFACEMETHODIMP CFileDialogEvents::OnOverwrite(IFileDialog * /*_Dialog*/, IShellItem * /*_Item*/, FDE_OVERWRITE_RESPONSE * /*_Response*/)
{
  return E_NOTIMPL;
}

//
// Implementation
//

bool CFileDialogEvents::IsInsideRoot(IShellItem & _Item) const
{
  std::filesystem::path ItemPath;
  return ReadShellItemPath(_Item, ItemPath) && FindRelativePath(ItemPath, m_RootFolder).has_value();
}

} // namespace editor::platform
