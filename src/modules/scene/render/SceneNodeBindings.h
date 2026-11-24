#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace editor
{

class ISceneNodeBinding;
struct TSceneBindingContext;

class CSceneNodeBindings final
{
public: // Types

  using TFactory = std::function<std::unique_ptr<ISceneNodeBinding>(const TSceneBindingContext &)>;

public: // Interface

  template <class TBinding>
  void Register(std::string_view _TypeId);

  std::unique_ptr<ISceneNodeBinding> Create(const std::string & _TypeId, const TSceneBindingContext & _Context) const;

private: // Implementation

  void Register(std::string _TypeId, TFactory _Factory);

private: // Attributes

  std::unordered_map<std::string, TFactory> m_Factories;
};

//
// Template implementation
//

template <class TBinding>
void CSceneNodeBindings::Register(std::string_view _TypeId)
{
  Register(std::string(_TypeId), [](const TSceneBindingContext & _Context)
  {
    return std::make_unique<TBinding>(_Context);
  });
}

} // namespace editor
