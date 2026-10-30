#include "AssetListView.h"
#include "modules/content_browser/presenter/ContentItem.h"

#include <imgui.h>

namespace editor
{

namespace
{

constexpr float RowHeightInFonts = 1.6f;
constexpr float RowPadding       = 4.0f;

} // namespace

//
// Constructor
//

CAssetListView::CAssetListView(IContentBrowserPresenter & _Presenter):
  CAssetItemsView(_Presenter)
{
  // Empty
}

//
// CAssetItemsView
//

void CAssetListView::DrawItems(const std::vector<TContentItem> & _Items)
{
  const float  RowHeight      = ImGui::GetFontSize() * RowHeightInFonts;
  const float  IconSize       = RowHeight - RowPadding * 2.0f;
  const float  AvailableWidth = ImGui::GetContentRegionAvail().x;
  const float  TypeWidth      = ImGui::CalcTextSize("Texture").x;
  const ImVec2 RowSize(AvailableWidth, RowHeight);

  ImGuiListClipper Clipper;
  Clipper.Begin(static_cast<int>(_Items.size()), RowHeight);

  while (Clipper.Step())
  {
    for (int RowIndex = Clipper.DisplayStart; RowIndex < Clipper.DisplayEnd; ++RowIndex)
    {
      const auto & Item        = _Items[static_cast<std::size_t>(RowIndex)];
      const ImVec2 RowPosition = ImGui::GetCursorScreenPos();
      const ImVec2 IconPosition(RowPosition.x + RowPadding, RowPosition.y + RowPadding);

      const float LabelX       = IconPosition.x + IconSize + RowPadding * 2.0f;
      const float FieldHeight  = IsRenaming(Item) ? ImGui::GetFrameHeight() : ImGui::GetFontSize();
      const float LabelWidth   = AvailableWidth - (LabelX - RowPosition.x) - TypeWidth - RowPadding * 2.0f;
      const ImVec2 LabelPosition(LabelX, RowPosition.y + (RowHeight - FieldHeight) * 0.5f);
      const ImVec2 TypePosition(RowPosition.x + AvailableWidth - TypeWidth - RowPadding, RowPosition.y + (RowHeight - ImGui::GetFontSize()) * 0.5f);

      ImGui::PushID(RowIndex);

      DrawItemButton(Item, RowPosition, RowSize);
      DrawItemIcon(Item, IconPosition, IconSize);
      ImGui::GetWindowDrawList()->AddText(TypePosition, ImGui::GetColorU32(ImGuiCol_TextDisabled), Item.TypeName.c_str());
      DrawItemLabel(Item, LabelPosition, LabelWidth, false);

      ImGui::PopID();

      ImGui::SetCursorScreenPos(RowPosition);
      ImGui::Dummy(RowSize);
    }
  }
}

} // namespace editor
