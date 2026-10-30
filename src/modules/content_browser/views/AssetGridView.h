#pragma once

#include "AssetItemsView.h"

namespace editor
{

class CAssetGridView final:
  public CAssetItemsView
{
public: // Constructor

  explicit CAssetGridView(IContentBrowserPresenter & _Presenter);

protected: // CAssetItemsView

  void DrawItems(const std::vector<TContentItem> & _Items) override;
};

} // namespace editor
