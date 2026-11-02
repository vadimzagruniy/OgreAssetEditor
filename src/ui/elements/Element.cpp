#include "Element.h"

#include <algorithm>
#include <utility>

namespace editor::ui
{

namespace
{

float ResolveExtent(const TSizeRule & _Rule, float _ContentExtent, float _AvailableExtent, float _LabelExtent)
{
  switch (_Rule.Mode)
  {
    case ESizeMode::Fixed: return _Rule.Value + _LabelExtent;
    case ESizeMode::Fill:  return _AvailableExtent;
    case ESizeMode::Auto:  break;
  }

  return _ContentExtent;
}

} // namespace

//
// Constructors/Destructor
//

CElement::CElement(std::string _Label):
  m_Label(std::move(_Label)),
  m_IsVisible(true),
  m_IsEnabled(true)
{
  // Empty
}

CElement::CElement(const CElement & _Other) = default;

CElement::CElement(CElement && _Other) noexcept = default;

CElement & CElement::operator=(const CElement & _Other) = default;

CElement & CElement::operator=(CElement && _Other) noexcept = default;

CElement::~CElement() = default;

//
// Interface
//

void CElement::Draw()
{
  if (!m_IsVisible)
    return;

  Draw(ResolveSize(CalcStyledContentSize(), ImGui::GetContentRegionAvail()));
}

void CElement::Draw(const ImVec2 & _Size)
{
  if (!m_IsVisible)
    return;

  PushStyle();
  ImGui::BeginDisabled(!m_IsEnabled);

  OnDraw(_Size);

  ImGui::EndDisabled();
  PopStyle();

  NotifyEditState();
  ShowTooltip();
}

ImVec2 CElement::CalcDesiredSize() const
{
  const ImVec2 ContentSize = CalcStyledContentSize();

  return ResolveSize(ContentSize, ContentSize);
}

bool CElement::IsVisible() const noexcept
{
  return m_IsVisible;
}

bool CElement::IsEnabled() const noexcept
{
  return m_IsEnabled;
}

const std::string & CElement::GetLabel() const noexcept
{
  return m_Label;
}

const TSizeRule & CElement::GetWidth() const noexcept
{
  return m_Width;
}

const TSizeRule & CElement::GetHeight() const noexcept
{
  return m_Height;
}

//
// Setup
//

void CElement::SetLabel(std::string _Label)
{
  m_Label = std::move(_Label);
}

void CElement::SetVisible(bool _IsVisible) noexcept
{
  m_IsVisible = _IsVisible;
}

void CElement::SetEnabled(bool _IsEnabled) noexcept
{
  m_IsEnabled = _IsEnabled;
}

void CElement::SetTooltip(std::string _Tooltip)
{
  m_Tooltip = std::move(_Tooltip);
}

void CElement::SetWidth(const TSizeRule & _Width) noexcept
{
  m_Width = _Width;
}

void CElement::SetHeight(const TSizeRule & _Height) noexcept
{
  m_Height = _Height;
}

void CElement::SetColor(ImGuiCol _Index, const ImVec4 & _Color)
{
  const auto Found = std::ranges::find(m_StyleColors, _Index, &TStyleColor::Index);
  if (Found != m_StyleColors.end())
  {
    Found->Color = _Color;
    return;
  }

  m_StyleColors.push_back({_Index, _Color});
}

void CElement::SetStyleVar(ImGuiStyleVar _Index, const TStyleVar::TValue & _Value)
{
  const auto Found = std::ranges::find(m_StyleVars, _Index, &TStyleVar::Index);
  if (Found != m_StyleVars.end())
  {
    Found->Value = _Value;
    return;
  }

  m_StyleVars.push_back({_Index, _Value});
}

void CElement::SetEditBeginCallback(std::function<void()> _Callback)
{
  m_EditBeginCallback = std::move(_Callback);
}

void CElement::SetEditEndCallback(std::function<void()> _Callback)
{
  m_EditEndCallback = std::move(_Callback);
}

//
// Derived interface
//

float CElement::CalcLabelExtent() const
{
  return 0.0f;
}

ImVec2 CElement::CalcLabelFrameSize() const
{
  const ImVec2   TextSize     = ImGui::CalcTextSize(m_Label.c_str(), nullptr, true);
  const ImVec2 & FramePadding = ImGui::GetStyle().FramePadding;

  return ImVec2(TextSize.x + FramePadding.x * 2.0f, TextSize.y + FramePadding.y * 2.0f);
}

float CElement::CalcSideLabelWidth() const
{
  const float TextWidth = ImGui::CalcTextSize(m_Label.c_str(), nullptr, true).x;
  if (TextWidth <= 0.0f)
    return 0.0f;

  return ImGui::GetStyle().ItemInnerSpacing.x + TextWidth;
}

ImVec2 CElement::CalcDefaultItemSize() const
{
  return ImVec2(ImGui::CalcItemWidth() + CalcSideLabelWidth(), ImGui::GetFrameHeight());
}

//
// Service
//

ImVec2 CElement::CalcStyledContentSize() const
{
  PushStyle();
  const ImVec2 ContentSize = CalcContentSize();
  PopStyle();

  return ContentSize;
}

ImVec2 CElement::ResolveSize(const ImVec2 & _ContentSize, const ImVec2 & _Available) const
{
  return ImVec2(ResolveExtent(m_Width, _ContentSize.x, _Available.x, CalcLabelExtent()), ResolveExtent(m_Height, _ContentSize.y, _Available.y, 0.0f));
}

void CElement::PushStyle() const
{
  for (const auto & StyleColor : m_StyleColors)
    ImGui::PushStyleColor(StyleColor.Index, StyleColor.Color);

  for (const auto & StyleVar : m_StyleVars)
  {
    std::visit([&StyleVar](const auto & _Value)
    {
      ImGui::PushStyleVar(StyleVar.Index, _Value);
    }, StyleVar.Value);
  }
}

void CElement::PopStyle() const
{
  ImGui::PopStyleVar(static_cast<int>(m_StyleVars.size()));
  ImGui::PopStyleColor(static_cast<int>(m_StyleColors.size()));
}

void CElement::ShowTooltip() const
{
  if (m_Tooltip.empty())
    return;

  ImGui::SetItemTooltip("%s", m_Tooltip.c_str());
}

void CElement::NotifyEditState() const
{
  if (m_EditBeginCallback && ImGui::IsItemActivated())
    m_EditBeginCallback();

  if (m_EditEndCallback && ImGui::IsItemDeactivatedAfterEdit())
    m_EditEndCallback();
}

} // namespace editor::ui
