#include "TimelineSerializer.h"
#include "editor/assets/AssetReferenceJson.h"
#include "editor/json/JsonFields.h"
#include "editor/timeline/model/Timeline.h"

#include <nlohmann/json.hpp>

#include <array>
#include <type_traits>
#include <utility>
#include <variant>

namespace editor
{

namespace
{

const std::string DurationKey      = "duration";
const std::string FrameRateKey     = "frameRate";
const std::string TracksKey        = "tracks";
const std::string TypeKey          = "type";
const std::string NameKey          = "name";
const std::string SlotKey          = "slot";
const std::string PropertyKey      = "property";
const std::string ValueTypeKey     = "valueType";
const std::string KeysKey          = "keys";
const std::string ClipsKey         = "clips";
const std::string ChildrenKey      = "children";
const std::string TimeKey          = "time";
const std::string ValueKey         = "value";
const std::string InterpolationKey = "interpolation";
const std::string StartKey         = "start";
const std::string LengthKey        = "length";
const std::string OffsetKey        = "offset";
const std::string SourceKey        = "source";

constexpr std::array<std::pair<ETimelineInterpolation, const char *>, 3> InterpolationNames
{{
  {ETimelineInterpolation::Step,   "step"},
  {ETimelineInterpolation::Linear, "linear"},
  {ETimelineInterpolation::Spline, "spline"}
}};

std::string GetInterpolationName(ETimelineInterpolation _Interpolation)
{
  for (const auto & [Interpolation, Name] : InterpolationNames)
  {
    if (Interpolation == _Interpolation)
      return Name;
  }

  return "linear";
}

bool ReadInterpolation(const TJson & _Object, ETimelineInterpolation & _Interpolation, std::string & _Error)
{
  std::string Name = GetInterpolationName(_Interpolation);
  if (!ReadString(_Object, InterpolationKey, Name, _Error))
    return false;

  for (const auto & [Interpolation, InterpolationName] : InterpolationNames)
  {
    if (Name == InterpolationName)
    {
      _Interpolation = Interpolation;
      return true;
    }
  }

  _Error = "Unknown interpolation '" + Name + "'.";
  return false;
}

std::string GetValueTypeName(const TPropertyValue & _Value)
{
  return std::visit([](const auto & _Alternative) -> std::string
  {
    using TAlternative = std::decay_t<decltype(_Alternative)>;

    if constexpr (std::is_same_v<TAlternative, bool>)
      return "bool";
    else if constexpr (std::is_same_v<TAlternative, float>)
      return "float";
    else if constexpr (std::is_same_v<TAlternative, std::size_t>)
      return "enum";
    else if constexpr (std::is_same_v<TAlternative, std::string>)
      return "text";
    else if constexpr (std::is_same_v<TAlternative, Ogre::Vector3>)
      return "vector3";
    else if constexpr (std::is_same_v<TAlternative, Ogre::Quaternion>)
      return "rotation";
    else if constexpr (std::is_same_v<TAlternative, Ogre::ColourValue>)
      return "color";
    else if constexpr (std::is_same_v<TAlternative, TAssetReference>)
      return "asset";
    else
      return {};
  }, _Value);
}

TJson WriteValue(const TPropertyValue & _Value)
{
  return std::visit([](const auto & _Alternative) -> TJson
  {
    using TAlternative = std::decay_t<decltype(_Alternative)>;

    if constexpr (std::is_same_v<TAlternative, std::monostate>)
      return nullptr;
    else if constexpr (std::is_same_v<TAlternative, Ogre::Vector3>)
      return WriteVector3(_Alternative);
    else if constexpr (std::is_same_v<TAlternative, Ogre::Quaternion>)
      return WriteQuaternion(_Alternative);
    else if constexpr (std::is_same_v<TAlternative, Ogre::ColourValue>)
      return WriteColour(_Alternative);
    else if constexpr (std::is_same_v<TAlternative, TAssetReference>)
      return WriteAssetReference(_Alternative);
    else
      return _Alternative;
  }, _Value);
}

template <class TValue, class TReader>
bool ReadTypedValue(const TJson & _Object, TValue _Initial, const TReader & _Read, TPropertyValue & _Value, std::string & _Error)
{
  TValue Value = std::move(_Initial);
  if (!_Read(_Object, ValueKey, Value, _Error))
    return false;

  _Value = std::move(Value);
  return true;
}

bool ReadEnumValue(const TJson & _Object, TPropertyValue & _Value, std::string & _Error)
{
  std::int64_t Index = 0;
  if (!ReadInt64(_Object, ValueKey, Index, _Error))
    return false;

  if (Index < 0)
  {
    _Error = "'value' must be a non-negative enum index.";
    return false;
  }

  _Value = static_cast<std::size_t>(Index);
  return true;
}

bool ReadValue(const TJson & _Object, const std::string & _ValueType, TPropertyValue & _Value, std::string & _Error)
{
  if (!_Object.contains(ValueKey))
    return true;

  if (_ValueType == "bool")
    return ReadTypedValue(_Object, false, ReadBool, _Value, _Error);

  if (_ValueType == "float")
    return ReadTypedValue(_Object, 0.0f, ReadFloat, _Value, _Error);

  if (_ValueType == "enum")
    return ReadEnumValue(_Object, _Value, _Error);

  if (_ValueType == "text")
    return ReadTypedValue(_Object, std::string(), ReadString, _Value, _Error);

  if (_ValueType == "vector3")
    return ReadTypedValue(_Object, Ogre::Vector3::ZERO, ReadVector3, _Value, _Error);

  if (_ValueType == "rotation")
    return ReadTypedValue(_Object, Ogre::Quaternion::IDENTITY, ReadQuaternion, _Value, _Error);

  if (_ValueType == "color")
    return ReadTypedValue(_Object, Ogre::ColourValue::White, ReadColour, _Value, _Error);

  if (_ValueType == "asset")
    return ReadTypedValue(_Object, TAssetReference(), ReadAssetReference, _Value, _Error);

  _Error = _ValueType.empty() ? "Key has a value, but the track has no 'valueType'." : "Unknown value type '" + _ValueType + "'.";
  return false;
}

bool CheckArray(const TJson & _Object, const std::string & _Key, std::string & _Error)
{
  const auto Field = _Object.find(_Key);
  if (Field == _Object.end() || Field->is_array())
    return true;

  _Error = "'" + _Key + "' must be an array.";
  return false;
}

bool CheckObject(const TJson & _Json, const std::string & _What, std::string & _Error)
{
  if (_Json.is_object())
    return true;

  _Error = _What + " must be a JSON object.";
  return false;
}

std::string FindValueType(const TTimelineTrack & _Track)
{
  for (const auto & Key : _Track.Keys)
  {
    auto ValueType = GetValueTypeName(Key.Value);
    if (!ValueType.empty())
      return ValueType;
  }

  return {};
}

TJson WriteKey(const TTimelineKey & _Key)
{
  TJson Json = TJson::object();
  Json[TimeKey] = _Key.Time;

  if (!std::holds_alternative<std::monostate>(_Key.Value))
    Json[ValueKey] = WriteValue(_Key.Value);

  Json[InterpolationKey] = GetInterpolationName(_Key.Interpolation);

  return Json;
}

TJson WriteClip(const TTimelineClip & _Clip)
{
  TJson Json = TJson::object();
  Json[StartKey]  = _Clip.Start;
  Json[LengthKey] = _Clip.Length;
  Json[OffsetKey] = _Clip.Offset;
  Json[SourceKey] = _Clip.Source;

  return Json;
}

TJson WriteTrack(const TTimelineTrack & _Track)
{
  TJson Json = TJson::object();
  Json[TypeKey] = _Track.TypeId;
  Json[NameKey] = _Track.Name;

  if (!_Track.Target.Slot.empty())
    Json[SlotKey] = _Track.Target.Slot;

  if (!_Track.Target.Property.empty())
    Json[PropertyKey] = _Track.Target.Property;

  if (const auto ValueType = FindValueType(_Track); !ValueType.empty())
    Json[ValueTypeKey] = ValueType;

  if (!_Track.Keys.empty())
  {
    auto & Keys = Json[KeysKey] = TJson::array();
    for (const auto & Key : _Track.Keys)
      Keys.push_back(WriteKey(Key));
  }

  if (!_Track.Clips.empty())
  {
    auto & Clips = Json[ClipsKey] = TJson::array();
    for (const auto & Clip : _Track.Clips)
      Clips.push_back(WriteClip(Clip));
  }

  if (!_Track.Children.empty())
  {
    auto & Children = Json[ChildrenKey] = TJson::array();
    for (const auto & Child : _Track.Children)
      Children.push_back(WriteTrack(Child));
  }

  return Json;
}

bool ReadKey(const TJson & _Json, const std::string & _ValueType, CTimeline & _Timeline, TTimelineKey & _Key, std::string & _Error)
{
  if (!CheckObject(_Json, "Key", _Error))
    return false;

  _Key.Id = _Timeline.MakeId();

  return ReadInt64(_Json, TimeKey, _Key.Time, _Error) &&
         ReadValue(_Json, _ValueType, _Key.Value, _Error) &&
         ReadInterpolation(_Json, _Key.Interpolation, _Error);
}

bool ReadClip(const TJson & _Json, CTimeline & _Timeline, TTimelineClip & _Clip, std::string & _Error)
{
  if (!CheckObject(_Json, "Clip", _Error))
    return false;

  _Clip.Id = _Timeline.MakeId();

  return ReadInt64(_Json, StartKey, _Clip.Start, _Error) &&
         ReadInt64(_Json, LengthKey, _Clip.Length, _Error) &&
         ReadInt64(_Json, OffsetKey, _Clip.Offset, _Error) &&
         ReadString(_Json, SourceKey, _Clip.Source, _Error);
}

bool ReadTrackFields(const TJson & _Json, TTimelineTrack & _Track, std::string & _ValueType, std::string & _Error)
{
  return ReadString(_Json, TypeKey, _Track.TypeId, _Error) &&
         ReadString(_Json, NameKey, _Track.Name, _Error) &&
         ReadString(_Json, SlotKey, _Track.Target.Slot, _Error) &&
         ReadString(_Json, PropertyKey, _Track.Target.Property, _Error) &&
         ReadString(_Json, ValueTypeKey, _ValueType, _Error) &&
         CheckArray(_Json, KeysKey, _Error) &&
         CheckArray(_Json, ClipsKey, _Error) &&
         CheckArray(_Json, ChildrenKey, _Error);
}

bool ReadTrack(const TJson & _Json, CTimeline & _Timeline, TTimelineTrack & _Track, std::string & _Error)
{
  if (!CheckObject(_Json, "Track", _Error))
    return false;

  std::string ValueType;
  if (!ReadTrackFields(_Json, _Track, ValueType, _Error))
    return false;

  _Track.Id = _Timeline.MakeId();

  for (const auto & KeyJson : _Json.value(KeysKey, TJson::array()))
  {
    TTimelineKey Key;
    if (!ReadKey(KeyJson, ValueType, _Timeline, Key, _Error))
      return false;

    _Track.Keys.push_back(std::move(Key));
  }

  for (const auto & ClipJson : _Json.value(ClipsKey, TJson::array()))
  {
    TTimelineClip Clip;
    if (!ReadClip(ClipJson, _Timeline, Clip, _Error))
      return false;

    _Track.Clips.push_back(std::move(Clip));
  }

  for (const auto & ChildJson : _Json.value(ChildrenKey, TJson::array()))
  {
    TTimelineTrack Child;
    if (!ReadTrack(ChildJson, _Timeline, Child, _Error))
    {
      _Error = "Track '" + Child.Name + "': " + _Error;
      return false;
    }

    _Track.Children.push_back(std::move(Child));
  }

  return true;
}

} // namespace

bool ReadTimeline(const TJson & _Json, CTimeline & _Timeline, std::string & _Error)
{
  if (!CheckObject(_Json, "Timeline", _Error))
    return false;

  CTimeline Timeline;

  TTimelineTicks Duration  = Timeline.GetDuration();
  std::int64_t   FrameRate = Timeline.GetFrameRate();

  if (!ReadInt64(_Json, DurationKey, Duration, _Error) || !ReadInt64(_Json, FrameRateKey, FrameRate, _Error) || !CheckArray(_Json, TracksKey, _Error))
    return false;

  Timeline.SetDuration(Duration);
  Timeline.SetFrameRate(static_cast<int>(FrameRate));

  for (const auto & TrackJson : _Json.value(TracksKey, TJson::array()))
  {
    TTimelineTrack Track;
    if (!ReadTrack(TrackJson, Timeline, Track, _Error))
    {
      _Error = "Track '" + Track.Name + "': " + _Error;
      return false;
    }

    Timeline.InsertTrack({InvalidTimelineId, Timeline.GetTracks().size()}, std::move(Track));
  }

  _Timeline = std::move(Timeline);
  return true;
}

TJson WriteTimeline(const CTimeline & _Timeline)
{
  TJson Json = TJson::object();
  Json[DurationKey]  = _Timeline.GetDuration();
  Json[FrameRateKey] = _Timeline.GetFrameRate();

  auto & Tracks = Json[TracksKey] = TJson::array();
  for (const auto & Track : _Timeline.GetTracks())
    Tracks.push_back(WriteTrack(Track));

  return Json;
}

} // namespace editor
