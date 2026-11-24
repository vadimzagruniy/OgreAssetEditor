#pragma once

#include "PropertyControlContext.h"
#include "ValuePropertyControl.h"
#include "ui/elements/ArrowButton.h"
#include "ui/elements/Button.h"
#include "ui/elements/InputText.h"
#include "ui/elements/Text.h"

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

namespace editor
{

class IAssetType;
struct TPropertyDesc;

class CAssetPropertyControl final:
  public CValuePropertyControl
{
public: // Constructor

  CAssetPropertyControl(const TPropertyDesc & _Desc, TPropertyControlContext _Context);

protected: // CValuePropertyControl

  void OnDraw(const TPropertyValue & _Value) override;

private: // Implementation

  void SetupElements();

  void SyncAppliedPath(const TPropertyValue & _Value);

  void DrawElements();

  void HandleDrop();

  void ApplyTypedPath();

  void Browse();

  void OpenAsset() const;

  void Assign(const std::string & _AssetPath);

  std::optional<std::string> ResolveAssetPath(const std::filesystem::path & _Path) const;

  bool IsMatchingType(const std::filesystem::path & _AssetPath) const;

  bool IsResolved() const;

  std::shared_ptr<IAssetType> FindAssetType() const;

  void Warn(const std::string & _Message) const;

private: // Attributes

  TPropertyControlContext    m_Context;
  std::string                m_AssetType;
  std::string                m_AppliedPath;
  std::optional<std::string> m_BrowsedPath;
  ui::CInputText             m_PathInput;
  ui::CButton                m_BrowseButton;
  ui::CArrowButton           m_OpenButton;
  ui::CText                  m_LabelText;
  bool                       m_IsReadOnly;
};

} // namespace editor
