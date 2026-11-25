#include "CreateAssetView.h"
#include "CreateAssetModal.h"
#include "modules/content_browser/presenter/IContentBrowserPresenter.h"
#include "ui/WindowManager.h"

#include <utility>

namespace editor
{

//
// Constructor
//

CCreateAssetView::CCreateAssetView(std::weak_ptr<IContentBrowserPresenter> _Presenter, ui::CWindowManager & _WindowManager):
  m_Presenter(std::move(_Presenter)),
  m_WindowManager(_WindowManager),
  m_IsModalOpened(false)
{
  // Empty
}

//
// Interface
//

void CCreateAssetView::Draw()
{
  const auto Presenter = m_Presenter.lock();
  if (!Presenter || !Presenter->GetCreationRequest())
  {
    m_IsModalOpened = false;
    return;
  }

  if (m_IsModalOpened)
    return;

  m_IsModalOpened = true;
  m_WindowManager.OpenModal(std::make_unique<CCreateAssetModal>(m_Presenter, *Presenter->GetCreationRequest()));
}

} // namespace editor
