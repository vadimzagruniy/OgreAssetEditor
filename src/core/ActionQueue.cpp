#include "ActionQueue.h"

#include <utility>

namespace editor::core
{

//
// Interface
//

void CActionQueue::Post(std::function<void()> _Action)
{
  m_Actions.push_back(std::move(_Action));
}

void CActionQueue::Apply()
{
  auto Actions = std::move(m_Actions);
  m_Actions.clear();

  for (const auto & Action : Actions)
    Action();
}

} // namespace editor::core
