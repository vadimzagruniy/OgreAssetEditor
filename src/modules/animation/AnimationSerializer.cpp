#include "AnimationSerializer.h"
#include "editor/json/Json.h"
#include "editor/timeline/TimelineSerializer.h"
#include "editor/timeline/model/Timeline.h"

#include <nlohmann/json.hpp>

#include <cstdint>

namespace editor
{

namespace
{

constexpr std::int64_t AnimationFormatVersion = 1;

const std::string VersionKey  = "version";
const std::string TimelineKey = "timeline";

} // namespace

bool ReadAnimation(const std::string & _Text, CTimeline & _Timeline, std::string & _Error)
{
  const auto Root = TJson::parse(_Text, nullptr, false);
  if (Root.is_discarded() || !Root.is_object())
  {
    _Error = "Animation file is not a valid JSON object.";
    return false;
  }

  const auto VersionField = Root.find(VersionKey);
  if (VersionField == Root.end() || !VersionField->is_number_integer())
  {
    _Error = "Animation file has no integer 'version'.";
    return false;
  }

  const auto Version = VersionField->get<std::int64_t>();
  if (Version < 1 || Version > AnimationFormatVersion)
  {
    _Error = "Animation format version " + std::to_string(Version) + " is not supported.";
    return false;
  }

  const auto TimelineField = Root.find(TimelineKey);
  if (TimelineField == Root.end())
  {
    _Timeline = CTimeline();
    return true;
  }

  return ReadTimeline(*TimelineField, _Timeline, _Error);
}

std::string WriteAnimation(const CTimeline & _Timeline)
{
  TJson Root = TJson::object();
  Root[VersionKey]  = AnimationFormatVersion;
  Root[TimelineKey] = WriteTimeline(_Timeline);

  return Root.dump(2, ' ', false, TJson::error_handler_t::replace) + "\n";
}

} // namespace editor
