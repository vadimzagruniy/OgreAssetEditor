#pragma once

#include "ModalChoice.h"

#include <functional>
#include <string>
#include <vector>

namespace editor::ui
{

struct TChoiceModalDesc
{
  std::string               Title;
  std::string               Message;
  std::vector<std::string>  Details;
  std::vector<TModalChoice> Choices;
  std::function<void()>     CancelAction;
};

} // namespace editor::ui
