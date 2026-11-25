#include "AnimationDocument.h"
#include "AnimationSerializer.h"
#include "editor/assets/AssetRecord.h"
#include "editor/documents/AssetProperty.h"

#include <format>
#include <fstream>
#include <utility>

namespace editor
{

namespace
{

std::size_t CountTracks(const std::vector<TTimelineTrack> & _Tracks)
{
  std::size_t Count = _Tracks.size();

  for (const auto & Track : _Tracks)
    Count += CountTracks(Track.Children);

  return Count;
}

} // namespace

//
// Constructor
//

CAnimationDocument::CAnimationDocument(TAssetRecord _Record):
  CTimelineDocument(std::move(_Record), "Animation")
{
  // Empty
}

//
// CAssetDocument
//

std::vector<TAssetProperty> CAnimationDocument::GetProperties() const
{
  const auto & Timeline = GetTimeline();
  const auto   Seconds  = static_cast<double>(Timeline.GetDuration()) / static_cast<double>(TimelineTicksPerSecond);

  return {{"Duration",   std::format("{:.2f} s", Seconds)                },
          {"Frame Rate", std::to_string(Timeline.GetFrameRate())         },
          {"Tracks",     std::to_string(CountTracks(Timeline.GetTracks()))}};
}

//
// Protected CAssetDocument
//

bool CAnimationDocument::OnSave(std::string & _Error)
{
  CommitPreview();

  std::ofstream File(GetRecord().AbsolutePath, std::ios::binary | std::ios::trunc);
  File << WriteAnimation(GetTimeline());
  File.close();

  if (!File)
  {
    _Error = "Could not write " + GetRecord().RelativePath.generic_string();
    return false;
  }

  return true;
}

//
// Interface
//

bool CAnimationDocument::Load(const std::string & _Text, std::string & _Error)
{
  CTimeline Timeline;
  if (!ReadAnimation(_Text, Timeline, _Error))
    return false;

  ResetTimeline(std::move(Timeline));
  return true;
}

} // namespace editor
