#pragma once

#include "AssetItemsView.h"

namespace editor
{

class CAssetListView final:
  public CAssetItemsView
{
public: // Constructor

  explicit CAssetListView(IContentBrowserPresenter & _Presenter);

protected: // CAssetItemsView

  void DrawItems(const std::vector<TContentItem> & _Items) override;
};

} // namespace editor
