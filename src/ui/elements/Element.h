#pragma once

#include "ui/elements/SizeRule.h"
#include "ui/elements/StyleColor.h"
#include "ui/elements/StyleVar.h"

#include <imgui.h>

#include <string>
#include <vector>

namespace editor::ui
{

class CElement
{
public: // Destructor

  virtual ~CElement();

protected: // Constructors

  explicit CElement(std::string _Label);

  CElement(const CElement & _Other);

  CElement(CElement && _Other) noexcept;

  CElement & operator=(const CElement & _Other);

  CElement & operator=(CElement && _Other) noexcept;

public: // Interface

  void Draw();

  void Draw(const ImVec2 & _Size);

  ImVec2 CalcDesiredSize() const;

  bool IsVisible() const noexcept;

  bool IsEnabled() const noexcept;

  const std::string & GetLabel() const noexcept;

  const TSizeRule & GetWidth() const noexcept;

  const TSizeRule & GetHeight() const noexcept;

protected: // Setup

  void SetLabel(std::string _Label);

  void SetVisible(bool _IsVisible) noexcept;

  void SetEnabled(bool _IsEnabled) noexcept;

  void SetTooltip(std::string _Tooltip);

  void SetWidth(const TSizeRule & _Width) noexcept;

  void SetHeight(const TSizeRule & _Height) noexcept;

  void SetColor(ImGuiCol _Index, const ImVec4 & _Color);

  void SetStyleVar(ImGuiStyleVar _Index, const TStyleVar::TValue & _Value);

protected: // Derived interface

  virtual ImVec2 CalcContentSize() const = 0;

  ImVec2 CalcLabelFrameSize() const;

  float CalcSideLabelWidth() const;

private: // Service

  ImVec2 CalcStyledContentSize() const;

  ImVec2 ResolveSize(const ImVec2 & _ContentSize, const ImVec2 & _Available) const;

  void PushStyle() const;

  void PopStyle() const;

  void ShowTooltip() const;

protected: // Events

  virtual void OnDraw(const ImVec2 & _Size) = 0;

private: // Attributes

  std::string              m_Label;
  std::string              m_Tooltip;
  TSizeRule                m_Width;
  TSizeRule                m_Height;
  std::vector<TStyleColor> m_StyleColors;
  std::vector<TStyleVar>   m_StyleVars;
  bool                     m_IsVisible;
  bool                     m_IsEnabled;
};

} // namespace editor::ui
