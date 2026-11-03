#include "MaterialPropertySource.h"
#include "modules/material/MaterialDocument.h"
#include "modules/material/MaterialInstance.h"
#include "modules/material/presenter/IMaterialEditorPresenter.h"

#include <cstddef>
#include <utility>

namespace editor
{

//
// Constructor
//

CMaterialPropertySource::CMaterialPropertySource(std::shared_ptr<IMaterialEditorPresenter> _Presenter):
  m_Presenter(std::move(_Presenter))
{
  // Empty
}

//
// IPropertySource
//

std::string CMaterialPropertySource::GetTitle() const
{
  const auto Document = GetDocument();
  if (!Document)
    return {};

  const int MaterialIndex = Document->GetSelection().MaterialIndex;
  if (MaterialIndex < 0)
    return {};

  return Document->GetInstance().GetMaterialName(static_cast<std::size_t>(MaterialIndex));
}

void CMaterialPropertySource::Commit()
{
  m_Presenter->CommitEdit();
}

//
// Service
//

std::shared_ptr<const CMaterialDocument> CMaterialPropertySource::GetDocument() const
{
  return m_Presenter->GetDocument();
}

IMaterialEditorPresenter & CMaterialPropertySource::GetPresenter() const noexcept
{
  return *m_Presenter;
}

} // namespace editor
