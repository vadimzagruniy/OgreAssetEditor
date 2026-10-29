#pragma once

#include <typeindex>
#include <typeinfo>

namespace editor::core
{

using TTypeKey = std::type_index;

template <class TType> TTypeKey TypeOf();

//
// Template implementation
//

template <class TType> TTypeKey TypeOf()
{
  return TTypeKey(typeid(TType));
}

} // namespace editor::core
