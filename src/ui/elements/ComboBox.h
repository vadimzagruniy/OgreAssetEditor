#pragma once

#include "ui/elements/ElementSetup.h"

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace editor::ui
{

class CComboBox final:
  public CElementSetup<CComboBox>
{
public: // Constructor

  explicit CComboBox(std::string _Label);

protected: // CElement

  ImVec2 CalcContentSize() const override;

  float CalcLabelExtent() const override;

  void OnDraw(const ImVec2 & _Size) override;

public: // Setup

  CComboBox & SetItems(std::vector<std::string> _Items);

  CComboBox & SetSelectedIndex(std::optional<std::size_t> _SelectedIndex) noexcept;

  CComboBox & SetPlaceholder(std::string _Placeholder);

  CComboBox & OnSelected(std::function<void(std::size_t)> _Callback);

public: // Interface

  const std::vector<std::string> & GetItems() const noexcept;

  std::optional<std::size_t> GetSelectedIndex() const noexcept;

private: // Service

  const std::string & GetPreview() const noexcept;

  std::optional<std::size_t> DrawItems() const;

private: // Attributes

  std::vector<std::string>         m_Items;
  std::string                      m_Placeholder;
  std::function<void(std::size_t)> m_SelectCallback;
  std::optional<std::size_t>       m_SelectedIndex;
};

} // namespace editor::ui
