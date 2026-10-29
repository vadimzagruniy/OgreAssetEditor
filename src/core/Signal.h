#pragma once

#include <functional>
#include <utility>
#include <vector>

namespace editor::core
{

template <class... TArgs> class CSignal
{
public: // Types

  using TSlot = std::function<void(TArgs...)>;

public: // Interface

  void Connect(TSlot _Slot);

  void Emit(TArgs... _Args) const;

private: // Attributes

  std::vector<TSlot> m_Slots;
};

//
// Template implementation
//

template <class... TArgs> void CSignal<TArgs...>::Connect(TSlot _Slot)
{
  m_Slots.push_back(std::move(_Slot));
}

template <class... TArgs> void CSignal<TArgs...>::Emit(TArgs... _Args) const
{
  for (const auto & Slot : m_Slots)
    Slot(_Args...);
}

} // namespace editor::core
