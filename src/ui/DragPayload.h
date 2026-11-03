#pragma once

#include <optional>
#include <string>

namespace editor::ui
{

struct TDropPayload;

class CDragPayload
{
public: // Constructor

  explicit CDragPayload(std::string _Type);

public: // Interface

  void Set(const std::string & _Data) const;

  std::optional<std::string> Accept() const;

  std::optional<TDropPayload> AcceptBeforeDelivery() const;

  std::optional<std::string> Peek() const;

private: // Attributes

  std::string m_Type;
};

} // namespace editor::ui
