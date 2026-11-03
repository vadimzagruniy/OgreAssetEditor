#include "DragPayload.h"
#include "DropPayload.h"

#include <imgui.h>

#include <cstddef>
#include <utility>

namespace editor::ui
{

namespace
{

std::string ReadData(const ImGuiPayload & _Payload)
{
  if (!_Payload.Data || _Payload.DataSize <= 0)
    return {};

  return std::string(static_cast<const char *>(_Payload.Data), static_cast<std::size_t>(_Payload.DataSize));
}

} // namespace

//
// Constructor
//

CDragPayload::CDragPayload(std::string _Type):
  m_Type(std::move(_Type))
{
  // Empty
}

//
// Interface
//

void CDragPayload::Set(const std::string & _Data) const
{
  if (_Data.empty())
    ImGui::SetDragDropPayload(m_Type.c_str(), nullptr, 0);
  else
    ImGui::SetDragDropPayload(m_Type.c_str(), _Data.data(), _Data.size());
}

std::optional<std::string> CDragPayload::Accept() const
{
  const auto * Payload = ImGui::AcceptDragDropPayload(m_Type.c_str());
  if (!Payload)
    return std::nullopt;

  return ReadData(*Payload);
}

std::optional<TDropPayload> CDragPayload::AcceptBeforeDelivery() const
{
  const auto * Payload = ImGui::AcceptDragDropPayload(m_Type.c_str(), ImGuiDragDropFlags_AcceptBeforeDelivery);
  if (!Payload)
    return std::nullopt;

  return TDropPayload{ReadData(*Payload), Payload->IsDelivery()};
}

std::optional<std::string> CDragPayload::Peek() const
{
  const auto * Payload = ImGui::GetDragDropPayload();
  if (!Payload || !Payload->IsDataType(m_Type.c_str()))
    return std::nullopt;

  return ReadData(*Payload);
}

} // namespace editor::ui
