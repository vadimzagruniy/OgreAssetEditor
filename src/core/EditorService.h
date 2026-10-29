#pragma once

#include "TypeKey.h"

#include <cassert>
#include <memory>
#include <unordered_map>

namespace editor::core
{

class CEditorService
{
public: // Constructor

  explicit CEditorService(const CEditorService * _Parent = nullptr) noexcept;

public: // Interface

  template <class TService> void Register(const std::shared_ptr<TService> & _Service);

  template <class TService> void Unregister();

  template <class TService> std::shared_ptr<TService> GetService() const;

  template <class TService> std::shared_ptr<TService> RequireService() const;

  template <class TService> bool HasService() const;

private: // Attributes

  const CEditorService *                            m_Parent;
  std::unordered_map<TTypeKey, std::weak_ptr<void>> m_Services;
};

//
// Template implementation
//

template <class TService> void CEditorService::Register(const std::shared_ptr<TService> & _Service)
{
  assert(_Service && "Cannot register an empty service");

  const auto [ServiceIterator, WasInserted] = m_Services.try_emplace(TypeOf<TService>(), std::weak_ptr<void>(_Service));

  assert(WasInserted && "Service already registered");
}

template <class TService> void CEditorService::Unregister()
{
  m_Services.erase(TypeOf<TService>());
}

template <class TService> std::shared_ptr<TService> CEditorService::GetService() const
{
  const auto ServiceIterator = m_Services.find(TypeOf<TService>());
  if (ServiceIterator != m_Services.end())
  {
    if (auto Service = ServiceIterator->second.lock())
      return std::static_pointer_cast<TService>(std::move(Service));
  }

  return m_Parent ? m_Parent->GetService<TService>() : nullptr;
}

template <class TService> std::shared_ptr<TService> CEditorService::RequireService() const
{
  auto Service = GetService<TService>();
  assert(Service && "Required service is missing");

  return Service;
}

template <class TService> bool CEditorService::HasService() const
{
  return static_cast<bool>(GetService<TService>());
}

} // namespace editor::core
