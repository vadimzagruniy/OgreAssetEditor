#include "TimelineKeyPropertySource.h"
#include "editor/inspector/PropertySchema.h"
#include "editor/timeline/TimelineDocument.h"
#include "editor/timeline/presenter/TimelineSelection.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <type_traits>
#include <utility>
#include <variant>

namespace editor
{

namespace
{

const std::string TimeProperty          = "time";
const std::string InterpolationProperty = "interpolation";
const std::string ValueProperty         = "value";

constexpr std::size_t ValueTypeCount = std::variant_size_v<TPropertyValue>;

EPropertyType GetValuePropertyType(std::size_t _ValueIndex)
{
  constexpr std::array<EPropertyType, ValueTypeCount> Types
  {
    EPropertyType::Group,
    EPropertyType::Bool,
    EPropertyType::Float,
    EPropertyType::Enum,
    EPropertyType::Text,
    EPropertyType::Vector3,
    EPropertyType::Rotation,
    EPropertyType::Color
  };

  return Types[_ValueIndex];
}

TPropertySchema BuildSchema(std::size_t _ValueIndex)
{
  TPropertySchema Schema;
  Schema.Identifier = "timeline.key." + std::to_string(_ValueIndex);

  TPropertyDesc Time{TimeProperty, "Time", EPropertyType::Float};
  Time.Meta.Minimum = 0.0f;
  Time.Meta.Maximum = 3600.0f;
  Time.Meta.Speed   = 0.01f;
  Time.Meta.Format  = "%.3f s";

  TPropertyDesc Interpolation{InterpolationProperty, "Interpolation", EPropertyType::Enum};
  Interpolation.Meta.Options = {"Step", "Linear", "Spline"};

  Schema.Properties = {std::move(Time), std::move(Interpolation)};

  const auto ValueType = GetValuePropertyType(_ValueIndex);
  if (ValueType == EPropertyType::Group)
    return Schema;

  TPropertyDesc Value{ValueProperty, "Value", ValueType};
  Value.Meta.IsReadOnly = ValueType == EPropertyType::Enum;

  Schema.Properties.push_back(std::move(Value));

  return Schema;
}

const TPropertySchema & GetKeySchema(const TPropertyValue & _Value)
{
  static const auto Schemas = []
  {
    std::array<TPropertySchema, ValueTypeCount> Result;

    for (std::size_t ValueIndex = 0; ValueIndex < ValueTypeCount; ++ValueIndex)
      Result[ValueIndex] = BuildSchema(ValueIndex);

    return Result;
  }();

  return Schemas[_Value.index()];
}

float ToSeconds(TTimelineTicks _Time)
{
  return static_cast<float>(static_cast<double>(_Time) / static_cast<double>(TimelineTicksPerSecond));
}

TTimelineTicks ToTicks(float _Seconds)
{
  return std::max<TTimelineTicks>(std::llround(static_cast<double>(_Seconds) * static_cast<double>(TimelineTicksPerSecond)), 0);
}

} // namespace

//
// Constructor
//

CTimelineKeyPropertySource::CTimelineKeyPropertySource(std::shared_ptr<CTimelineDocument> _Document, std::shared_ptr<const CTimelineSelection> _Selection):
  m_Document(std::move(_Document)),
  m_Selection(std::move(_Selection))
{
  // Empty
}

//
// IPropertySource
//

std::string CTimelineKeyPropertySource::GetTitle() const
{
  const auto & Timeline = m_Document->GetTimeline();
  const auto   Key      = FindSelectedKey();
  const auto   TrackId  = Key ? Timeline.FindItemTrack(Key->Id) : std::nullopt;

  if (!TrackId)
    return "Key";

  const auto & Track = Timeline.GetTrack(*TrackId);
  return (Track.Name.empty() ? Track.TypeId : Track.Name) + " Key";
}

const TPropertySchema & CTimelineKeyPropertySource::GetSchema() const
{
  const auto Key = FindSelectedKey();
  return GetKeySchema(Key ? Key->Value : TPropertyValue());
}

TPropertyValue CTimelineKeyPropertySource::GetValue(const std::string & _Identifier) const
{
  const auto Key = FindSelectedKey();
  if (!Key)
    return {};

  if (_Identifier == TimeProperty)
    return ToSeconds(Key->Time);

  if (_Identifier == InterpolationProperty)
    return static_cast<std::size_t>(Key->Interpolation);

  if (_Identifier == ValueProperty)
    return Key->Value;

  return {};
}

void CTimelineKeyPropertySource::Preview(const std::string & _Identifier, const TPropertyValue & _Value)
{
  const auto Key = FindSelectedKey();
  if (!Key)
    return;

  m_Document->PreviewKey(Key->Id, [&_Identifier, &_Value](TTimelineKey & _Key)
  {
    if (_Identifier == TimeProperty && std::holds_alternative<float>(_Value))
      _Key.Time = ToTicks(std::get<float>(_Value));
    else
    if (_Identifier == InterpolationProperty && std::holds_alternative<std::size_t>(_Value))
      _Key.Interpolation = static_cast<ETimelineInterpolation>(std::min<std::size_t>(std::get<std::size_t>(_Value), 2));
    else
    if (_Identifier == ValueProperty && _Value.index() == _Key.Value.index())
      _Key.Value = _Value;
  });
}

void CTimelineKeyPropertySource::Commit()
{
  m_Document->CommitPreview();
}

//
// Interface
//

bool CTimelineKeyPropertySource::HasKey() const
{
  return FindSelectedKey().has_value();
}

//
// Implementation
//

std::optional<TTimelineKey> CTimelineKeyPropertySource::FindSelectedKey() const
{
  const auto & Items = m_Selection->GetItems();
  if (Items.size() != 1)
    return std::nullopt;

  const auto   KeyId    = *Items.begin();
  const auto & Timeline = m_Document->GetTimeline();
  const auto   TrackId  = Timeline.FindItemTrack(KeyId);

  if (!TrackId)
    return std::nullopt;

  const auto & Keys = Timeline.GetTrack(*TrackId).Keys;
  const auto   Key  = std::find_if(Keys.begin(), Keys.end(), [KeyId](const TTimelineKey & _Key)
  {
    return _Key.Id == KeyId;
  });

  if (Key == Keys.end())
    return std::nullopt;

  return *Key;
}

} // namespace editor
