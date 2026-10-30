#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <utility>
#include <vector>

namespace editor::core
{

template <class... TArgs> class CSignal
{
public: // Types

  using TSlot         = std::function<void(TArgs...)>;
  using TConnectionId = std::size_t;

public: // Interface

  TConnectionId Connect(TSlot _Slot);

  void Disconnect(TConnectionId _ConnectionId);

  void Emit(TArgs... _Args) const;

private: // Types

  struct TConnection
  {
    TConnectionId Id;
    TSlot         Slot;
  };

private: // Attributes

  std::vector<TConnection> m_Connections;
  TConnectionId            m_NextConnectionId = 1;
};

//
// Template implementation
//

template <class... TArgs> typename CSignal<TArgs...>::TConnectionId CSignal<TArgs...>::Connect(TSlot _Slot)
{
  const TConnectionId ConnectionId = m_NextConnectionId++;
  m_Connections.push_back({ConnectionId, std::move(_Slot)});

  return ConnectionId;
}

template <class... TArgs> void CSignal<TArgs...>::Disconnect(TConnectionId _ConnectionId)
{
  std::erase_if(m_Connections, [_ConnectionId](const TConnection & _Connection)
  {
    return _Connection.Id == _ConnectionId;
  });
}

template <class... TArgs> void CSignal<TArgs...>::Emit(TArgs... _Args) const
{
  for (const auto & Connection : m_Connections)
    Connection.Slot(_Args...);
}

} // namespace editor::core
