#pragma once

#include "ChoiceModalDesc.h"
#include "ModalWindow.h"

#include <cstddef>
#include <string>

namespace editor::ui
{

class CChoiceModal final:
  public CModalWindow
{
public: // Constructor

  explicit CChoiceModal(TChoiceModalDesc _Desc);

protected: // CModalWindow

  void OnContentGui() override;

  void OnCancel() override;

private: // Events

  void OnChoiceClicked(std::size_t _ChoiceIndex);

private: // Attributes

  TChoiceModalDesc m_Desc;
  std::string      m_Error;
};

} // namespace editor::ui
