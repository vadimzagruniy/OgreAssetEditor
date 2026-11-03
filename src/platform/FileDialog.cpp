#include "FileDialog.h"
#include "FileDialogDesc.h"
#include "FileDialogEvents.h"
#include "ShellItem.h"

#include <windows.h>
#include <shobjidl.h>
#include <wrl/client.h>

#include <system_error>

namespace editor::platform
{

namespace
{

using Microsoft::WRL::ComPtr;

std::filesystem::path NormalizePath(const std::filesystem::path & _Path)
{
  std::error_code ErrorCode;
  const auto Canonical = std::filesystem::weakly_canonical(_Path, ErrorCode);

  return ErrorCode ? _Path.lexically_normal() : Canonical;
}

std::wstring MakeFilterSpec(const TFileDialogDesc & _Desc)
{
  std::wstring Spec;

  for (const auto & Extension : _Desc.Extensions)
  {
    if (!Spec.empty())
      Spec += L';';

    Spec += L"*" + std::filesystem::path(Extension).wstring();
  }

  return Spec.empty() ? L"*.*" : Spec;
}

bool SetupDialog(IFileOpenDialog & _Dialog, const TFileDialogDesc & _Desc, std::string & _Error)
{
  FILEOPENDIALOGOPTIONS Options = 0;
  _Dialog.GetOptions(&Options);
  _Dialog.SetOptions(Options | FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST | FOS_NOCHANGEDIR);

  const std::wstring      FilterName = std::filesystem::path(_Desc.FilterName).wstring();
  const std::wstring      FilterSpec = MakeFilterSpec(_Desc);
  const COMDLG_FILTERSPEC FileType{FilterName.c_str(), FilterSpec.c_str()};

  _Dialog.SetFileTypes(1, &FileType);
  _Dialog.SetTitle(std::filesystem::path(_Desc.Title).wstring().c_str());

  const auto InitialFolder = FindRelativePath(_Desc.InitialFolder, _Desc.RootFolder) ? _Desc.InitialFolder : _Desc.RootFolder;

  ComPtr<IShellItem> Folder;
  if (FAILED(SHCreateItemFromParsingName(NormalizePath(InitialFolder).c_str(), nullptr, IID_PPV_ARGS(&Folder))))
  {
    _Error = "Folder '" + InitialFolder.string() + "' cannot be opened.";
    return false;
  }

  _Dialog.SetFolder(Folder.Get());

  return true;
}

bool ShowDialog(const TFileDialogDesc & _Desc, std::filesystem::path & _Selected, std::string & _Error)
{
  ComPtr<IFileOpenDialog> Dialog;
  if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&Dialog))))
  {
    _Error = "File dialog is not available.";
    return false;
  }

  if (!SetupDialog(*Dialog.Get(), _Desc, _Error))
    return false;

  CFileDialogEvents Events(_Desc.RootFolder);
  DWORD             Cookie = 0;
  Dialog->Advise(&Events, &Cookie);

  const HRESULT ShowResult = Dialog->Show(GetActiveWindow());

  Dialog->Unadvise(Cookie);

  if (FAILED(ShowResult))
  {
    if (ShowResult != HRESULT_FROM_WIN32(ERROR_CANCELLED))
      _Error = "File dialog failed.";

    return false;
  }

  ComPtr<IShellItem> Result;
  if (FAILED(Dialog->GetResult(&Result)) || !ReadShellItemPath(*Result.Get(), _Selected))
  {
    _Error = "Selected file cannot be read.";
    return false;
  }

  return true;
}

} // namespace

bool OpenFileDialog(const TFileDialogDesc & _Desc, std::filesystem::path & _Selected, std::string & _Error)
{
  const HRESULT InitResult = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
  if (FAILED(InitResult) && InitResult != RPC_E_CHANGED_MODE)
  {
    _Error = "COM cannot be initialized.";
    return false;
  }

  const bool IsSelected = ShowDialog(_Desc, _Selected, _Error);

  if (SUCCEEDED(InitResult))
    CoUninitialize();

  return IsSelected;
}

std::optional<std::filesystem::path> FindRelativePath(const std::filesystem::path & _Path, const std::filesystem::path & _Folder)
{
  if (_Path.empty() || _Folder.empty())
    return std::nullopt;

  const auto Relative = NormalizePath(_Path).lexically_relative(NormalizePath(_Folder));
  if (Relative.empty() || *Relative.begin() == "..")
    return std::nullopt;

  return Relative;
}

} // namespace editor::platform
