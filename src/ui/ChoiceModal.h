#pragma once

#include "ChoiceModalDesc.h"
#include "ModalWindow.h"

#include <string>

namespace editor::ui
{

class CChoiceModal final:
  public CModalWindow
{
public: // Constructor

  explicit CChoiceModal(TChoiceModalDesc _Desc);

protected: // CModalWindow

  void OnGui() override;

  void OnCancel() override;

private: // Attributes

  TChoiceModalDesc m_Desc;
  std::string      m_Error;
};

} // namespace editor::ui
