#pragma once

#include "ui/elements/ElementSetup.h"
#include "ui/layout/LayoutAlign.h"
#include "ui/layout/LayoutDirection.h"

#include <cstddef>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

namespace editor::ui
{

class CLayout final:
  public CElementSetup<CLayout>
{
public: // Constructor

  explicit CLayout(ELayoutDirection _Direction = ELayoutDirection::Row, ELayoutAlign _Align = ELayoutAlign::Start);

protected: // CElement

  ImVec2 CalcContentSize() const override;

  void OnDraw(const ImVec2 & _Size) override;

public: // Interface

  template <class TElement>
  std::decay_t<TElement> & Add(TElement && _Element);

  void Clear() noexcept;

  ELayoutDirection GetDirection() const noexcept;

public: // Setup

  CLayout & SetDirection(ELayoutDirection _Direction) noexcept;

  CLayout & SetAlign(ELayoutAlign _Align) noexcept;

  CLayout & SetCrossAlign(ELayoutAlign _CrossAlign) noexcept;

  CLayout & SetSpacing(float _Spacing) noexcept;

  CLayout & SetPadding(const ImVec2 & _Padding) noexcept;

private: // Service

  float ArrangeElements(const ImVec2 & _InnerSize);

  void DrawElements(const ImVec2 & _Start, const ImVec2 & _InnerSize, float _FreeSpace);

  std::size_t CountVisibleElements() const;

  float CalcTotalSpacing(std::size_t _VisibleCount) const;

  float GetSpacing() const;

  ELayoutAlign GetCrossAlign() const noexcept;

  std::size_t GetMainAxis() const noexcept;

  std::size_t GetCrossAxis() const noexcept;

private: // Attributes

  std::vector<std::unique_ptr<CElement>> m_Elements;
  std::vector<ImVec2>                    m_ElementSizes;
  ImVec2                                 m_Padding;
  std::optional<float>                   m_Spacing;
  ELayoutDirection                       m_Direction;
  ELayoutAlign                           m_Align;
  std::optional<ELayoutAlign>            m_CrossAlign;
};

//
// Template implementation
//

template <class TElement>
std::decay_t<TElement> & CLayout::Add(TElement && _Element)
{
  using TValue = std::decay_t<TElement>;
  static_assert(std::is_base_of_v<CElement, TValue>, "Layout accepts only UI elements");

  auto   Element   = std::make_unique<TValue>(std::forward<TElement>(_Element));
  auto & Reference = *Element;

  m_Elements.push_back(std::move(Element));

  return Reference;
}

} // namespace editor::ui
