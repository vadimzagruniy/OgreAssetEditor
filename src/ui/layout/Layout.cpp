#include "Layout.h"

#include <algorithm>

namespace editor::ui
{

namespace
{

const TSizeRule & GetAxisRule(const CElement & _Element, std::size_t _Axis)
{
  return _Axis == 0 ? _Element.GetWidth() : _Element.GetHeight();
}

float CalcAlignOffset(ELayoutAlign _Align, float _FreeSpace)
{
  switch (_Align)
  {
    case ELayoutAlign::Center: return _FreeSpace * 0.5f;
    case ELayoutAlign::End:    return _FreeSpace;
    case ELayoutAlign::Start:  break;
  }

  return 0.0f;
}

} // namespace

//
// Constructor
//

CLayout::CLayout(ELayoutDirection _Direction, ELayoutAlign _Align):
  CElementSetup(std::string()),
  m_Padding(0.0f, 0.0f),
  m_Direction(_Direction),
  m_Align(_Align)
{
  SetWidth({ESizeMode::Fill});
}

//
// CElement
//

ImVec2 CLayout::CalcContentSize() const
{
  const std::size_t MainAxis  = GetMainAxis();
  const std::size_t CrossAxis = GetCrossAxis();

  ImVec2 ContentSize(0.0f, 0.0f);

  for (const auto & Element : m_Elements)
  {
    if (!Element->IsVisible())
      continue;

    const ImVec2 ElementSize = Element->CalcDesiredSize();

    ContentSize[MainAxis]  += ElementSize[MainAxis];
    ContentSize[CrossAxis]  = std::max(ContentSize[CrossAxis], ElementSize[CrossAxis]);
  }

  ContentSize[MainAxis] += CalcTotalSpacing(CountVisibleElements());

  return ImVec2(ContentSize.x + m_Padding.x * 2.0f, ContentSize.y + m_Padding.y * 2.0f);
}

void CLayout::OnDraw(const ImVec2 & _Size)
{
  const ImVec2 Origin = ImGui::GetCursorScreenPos();
  const ImVec2 Start(Origin.x + m_Padding.x, Origin.y + m_Padding.y);
  const ImVec2 InnerSize(std::max(_Size.x - m_Padding.x * 2.0f, 0.0f), std::max(_Size.y - m_Padding.y * 2.0f, 0.0f));

  const float FreeSpace = ArrangeElements(InnerSize);
  DrawElements(Start, InnerSize, FreeSpace);

  ImGui::SetCursorScreenPos(Origin);
  ImGui::Dummy(_Size);
}

//
// Setup
//

CLayout & CLayout::SetDirection(ELayoutDirection _Direction) noexcept
{
  m_Direction = _Direction;
  return *this;
}

CLayout & CLayout::SetAlign(ELayoutAlign _Align) noexcept
{
  m_Align = _Align;
  return *this;
}

CLayout & CLayout::SetCrossAlign(ELayoutAlign _CrossAlign) noexcept
{
  m_CrossAlign = _CrossAlign;
  return *this;
}

CLayout & CLayout::SetSpacing(float _Spacing) noexcept
{
  m_Spacing = _Spacing;
  return *this;
}

CLayout & CLayout::SetPadding(const ImVec2 & _Padding) noexcept
{
  m_Padding = _Padding;
  return *this;
}

//
// Service
//

float CLayout::ArrangeElements(const ImVec2 & _InnerSize)
{
  const std::size_t MainAxis  = GetMainAxis();
  const std::size_t CrossAxis = GetCrossAxis();

  m_ElementSizes.assign(m_Elements.size(), ImVec2(0.0f, 0.0f));

  float       UsedSpace = CalcTotalSpacing(CountVisibleElements());
  std::size_t FillCount = 0;

  for (std::size_t ElementIndex = 0; ElementIndex < m_Elements.size(); ++ElementIndex)
  {
    const auto & Element = *m_Elements[ElementIndex];
    if (!Element.IsVisible())
      continue;

    ImVec2 & ElementSize = m_ElementSizes[ElementIndex];
    ElementSize = Element.CalcDesiredSize();

    if (GetAxisRule(Element, CrossAxis).Mode == ESizeMode::Fill)
      ElementSize[CrossAxis] = _InnerSize[CrossAxis];

    if (GetAxisRule(Element, MainAxis).Mode == ESizeMode::Fill)
      ++FillCount;
    else
      UsedSpace += ElementSize[MainAxis];
  }

  const float FreeSpace = std::max(_InnerSize[MainAxis] - UsedSpace, 0.0f);
  if (FillCount == 0)
    return FreeSpace;

  const float FillExtent = FreeSpace / static_cast<float>(FillCount);

  for (std::size_t ElementIndex = 0; ElementIndex < m_Elements.size(); ++ElementIndex)
  {
    const auto & Element = *m_Elements[ElementIndex];
    if (Element.IsVisible() && GetAxisRule(Element, MainAxis).Mode == ESizeMode::Fill)
      m_ElementSizes[ElementIndex][MainAxis] = FillExtent;
  }

  return 0.0f;
}

void CLayout::DrawElements(const ImVec2 & _Start, const ImVec2 & _InnerSize, float _FreeSpace)
{
  const std::size_t  MainAxis   = GetMainAxis();
  const std::size_t  CrossAxis  = GetCrossAxis();
  const ELayoutAlign CrossAlign = GetCrossAlign();
  const float        Spacing    = GetSpacing();

  ImVec2 Cursor = _Start;
  Cursor[MainAxis] += CalcAlignOffset(m_Align, _FreeSpace);

  for (std::size_t ElementIndex = 0; ElementIndex < m_Elements.size(); ++ElementIndex)
  {
    auto & Element = *m_Elements[ElementIndex];
    if (!Element.IsVisible())
      continue;

    const ImVec2 & ElementSize = m_ElementSizes[ElementIndex];

    ImVec2 Position = Cursor;
    Position[CrossAxis] += CalcAlignOffset(CrossAlign, std::max(_InnerSize[CrossAxis] - ElementSize[CrossAxis], 0.0f));

    ImGui::SetCursorScreenPos(Position);
    Element.Draw(ElementSize);

    Cursor[MainAxis] += ElementSize[MainAxis] + Spacing;
  }
}

std::size_t CLayout::CountVisibleElements() const
{
  return static_cast<std::size_t>(std::ranges::count_if(m_Elements, [](const auto & _Element)
  {
    return _Element->IsVisible();
  }));
}

float CLayout::CalcTotalSpacing(std::size_t _VisibleCount) const
{
  if (_VisibleCount < 2)
    return 0.0f;

  return GetSpacing() * static_cast<float>(_VisibleCount - 1);
}

float CLayout::GetSpacing() const
{
  return m_Spacing.value_or(ImGui::GetStyle().ItemSpacing[GetMainAxis()]);
}

ELayoutAlign CLayout::GetCrossAlign() const noexcept
{
  if (m_CrossAlign)
    return *m_CrossAlign;

  return m_Direction == ELayoutDirection::Row ? ELayoutAlign::Center : ELayoutAlign::Start;
}

std::size_t CLayout::GetMainAxis() const noexcept
{
  return m_Direction == ELayoutDirection::Row ? 0 : 1;
}

std::size_t CLayout::GetCrossAxis() const noexcept
{
  return 1 - GetMainAxis();
}

} // namespace editor::ui
