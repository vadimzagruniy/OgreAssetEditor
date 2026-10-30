#include "AssetGridView.h"
#include "modules/content_browser/presenter/ContentItem.h"

#include <imgui.h>

#include <algorithm>

namespace editor
{

namespace
{

constexpr float IconSizeInFonts  = 4.0f;
constexpr float TileWidthInIcons = 1.5f;
constexpr float TilePadding      = 6.0f;

} // namespace

//
// Constructor
//

CAssetGridView::CAssetGridView(IContentBrowserPresenter & _Presenter):
  CAssetItemsView(_Presenter)
{
  // Empty
}

//
// CAssetItemsView
//

void CAssetGridView::DrawItems(const std::vector<TContentItem> & _Items)
{
  const float  IconSize       = ImGui::GetFontSize() * IconSizeInFonts;
  const ImVec2 TileSize(IconSize * TileWidthInIcons, IconSize + ImGui::GetFrameHeight() + TilePadding * 3.0f);
  const float  LabelWidth     = TileSize.x - TilePadding * 2.0f;
  const float  AvailableWidth = ImGui::GetContentRegionAvail().x;
  const int    ColumnCount    = std::max(1, static_cast<int>(AvailableWidth / TileSize.x));
  const int    ItemCount      = static_cast<int>(_Items.size());
  const int    RowCount       = (ItemCount + ColumnCount - 1) / ColumnCount;

  ImGuiListClipper Clipper;
  Clipper.Begin(RowCount, TileSize.y);

  while (Clipper.Step())
  {
    for (int RowIndex = Clipper.DisplayStart; RowIndex < Clipper.DisplayEnd; ++RowIndex)
    {
      const ImVec2 RowPosition = ImGui::GetCursorScreenPos();

      for (int ColumnIndex = 0; ColumnIndex < ColumnCount; ++ColumnIndex)
      {
        const int ItemIndex = RowIndex * ColumnCount + ColumnIndex;
        if (ItemIndex >= ItemCount)
          break;

        const auto & Item         = _Items[static_cast<std::size_t>(ItemIndex)];
        const ImVec2 TilePosition(RowPosition.x + TileSize.x * static_cast<float>(ColumnIndex), RowPosition.y);
        const ImVec2 IconPosition(TilePosition.x + (TileSize.x - IconSize) * 0.5f, TilePosition.y + TilePadding);
        const ImVec2 LabelPosition(TilePosition.x + TilePadding, IconPosition.y + IconSize + TilePadding);

        ImGui::PushID(ItemIndex);

        DrawItemButton(Item, TilePosition, TileSize);
        DrawItemIcon(Item, IconPosition, IconSize);
        DrawItemLabel(Item, LabelPosition, LabelWidth, true);

        ImGui::PopID();
      }

      ImGui::SetCursorScreenPos(RowPosition);
      ImGui::Dummy(ImVec2(AvailableWidth, TileSize.y));
    }
  }
}

} // namespace editor
