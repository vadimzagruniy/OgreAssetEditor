#include "AssetReferencesView.h"
#include "modules/content_browser/presenter/IContentBrowserPresenter.h"
#include "ui/ChoiceModal.h"
#include "ui/ChoiceModalDesc.h"
#include "ui/WindowManager.h"

#include <string>
#include <utility>

namespace editor
{

//
// Constructor
//

CAssetReferencesView::CAssetReferencesView(std::weak_ptr<IContentBrowserPresenter> _Presenter, ui::CWindowManager & _WindowManager):
  m_Presenter(std::move(_Presenter)),
  m_WindowManager(_WindowManager),
  m_IsModalOpened(false)
{
  // Empty
}

//
// Interface
//

void CAssetReferencesView::Draw()
{
  const auto Presenter = m_Presenter.lock();
  if (!Presenter || !Presenter->GetReferencesReport())
  {
    m_IsModalOpened = false;
    return;
  }

  if (m_IsModalOpened)
    return;

  m_IsModalOpened = true;
  OpenModal(*Presenter);
}

//
// Implementation
//

void CAssetReferencesView::OpenModal(const IContentBrowserPresenter & _Presenter)
{
  const auto & Report = *_Presenter.GetReferencesReport();

  ui::TChoiceModalDesc Desc;
  Desc.Title   = "References";
  Desc.Size    = ImVec2(40.0f, 12.0f);
  Desc.Details = Report.ReferrerPaths;
  Desc.Message = Report.ReferrerPaths.empty()
    ? "No assets use " + Report.AssetPath + "."
    : std::to_string(Report.ReferrerPaths.size()) + " asset(s) use " + Report.AssetPath + ":";

  auto Close = [Presenter = m_Presenter]
  {
    if (const auto LockedPresenter = Presenter.lock())
      LockedPresenter->CloseReferencesReport();
  };

  Desc.Choices =
  {
    {"Close", [Close](std::string & /*_Error*/)
    {
      Close();
      return true;
    }}
  };

  Desc.CancelAction = Close;

  m_WindowManager.OpenModal(std::make_unique<ui::CChoiceModal>(std::move(Desc)));
}

} // namespace editor
