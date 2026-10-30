#include "DeleteConfirmationView.h"
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

CDeleteConfirmationView::CDeleteConfirmationView(std::weak_ptr<IContentBrowserPresenter> _Presenter, ui::CWindowManager & _WindowManager):
  m_Presenter(std::move(_Presenter)),
  m_WindowManager(_WindowManager),
  m_IsModalOpened(false)
{
  // Empty
}

//
// Interface
//

void CDeleteConfirmationView::Draw()
{
  const auto Presenter = m_Presenter.lock();
  if (!Presenter || !Presenter->IsDeleteRequested())
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

void CDeleteConfirmationView::OpenModal(const IContentBrowserPresenter & _Presenter)
{
  const auto & Selection = _Presenter.GetSelection();

  ui::TChoiceModalDesc Desc;
  Desc.Title   = "Delete Assets";
  Desc.Message = "Delete " + std::to_string(Selection.size()) + " item(s)? This cannot be undone.";

  for (const auto & Path : Selection)
    Desc.Details.push_back(Path.generic_string());

  Desc.Choices =
  {
    {"Delete", [Presenter = m_Presenter](std::string & /*_Error*/)
    {
      if (const auto LockedPresenter = Presenter.lock())
        LockedPresenter->ConfirmDelete();

      return true;
    }},
    {"Cancel", [Presenter = m_Presenter](std::string & /*_Error*/)
    {
      if (const auto LockedPresenter = Presenter.lock())
        LockedPresenter->CancelDelete();

      return true;
    }}
  };

  Desc.CancelAction = [Presenter = m_Presenter]
  {
    if (const auto LockedPresenter = Presenter.lock())
      LockedPresenter->CancelDelete();
  };

  m_WindowManager.OpenModal(std::make_unique<ui::CChoiceModal>(std::move(Desc)));
}

} // namespace editor
