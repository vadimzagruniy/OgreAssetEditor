#pragma once

#include "ModalChoice.h"

#include <imgui.h>

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
  ImVec2                    Size = ImVec2(36.0f, 11.0f);
};

} // namespace editor::ui
