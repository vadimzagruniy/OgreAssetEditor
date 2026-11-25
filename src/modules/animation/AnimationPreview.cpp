#include "AnimationPreview.h"
#include "editor/timeline/model/Timeline.h"
#include "editor/timeline/playback/TimelineKeyEvaluation.h"
#include "modules/scene/animation/ISceneAnimationHost.h"

#include <OgreSceneNode.h>

#include <utility>
#include <variant>

namespace editor
{

namespace
{

const std::string TransformTrackType  = "transform";
const std::string PositionProperty    = "position";
const std::string OrientationProperty = "orientation";
const std::string ScaleProperty       = "scale";

} // namespace

//
// Constructor
//

CAnimationPreview::CAnimationPreview(std::weak_ptr<ISceneAnimationHost> _Host):
  m_Host(std::move(_Host))
{
  // Empty
}

//
// ITimelineEvaluator
//

void CAnimationPreview::BeginPreview()
{
  m_TouchedNodes.clear();
}

void CAnimationPreview::Evaluate(const CTimeline & _Timeline, TTimelineTicks /*_From*/, TTimelineTicks _To, ETimelineEvaluateMode /*_Mode*/)
{
  const auto Host = m_Host.lock();
  if (!Host)
    return;

  EvaluateTracks(_Timeline.GetTracks(), _To, *Host);
}

void CAnimationPreview::EndPreview()
{
  if (const auto Host = m_Host.lock())
  {
    for (const auto & NodeName : m_TouchedNodes)
      Host->RestoreNode(NodeName);
  }

  m_TouchedNodes.clear();
}

//
// Implementation
//

void CAnimationPreview::EvaluateTracks(const std::vector<TTimelineTrack> & _Tracks, TTimelineTicks _Time, ISceneAnimationHost & _Host)
{
  for (const auto & Track : _Tracks)
  {
    if (Track.TypeId == TransformTrackType)
      ApplyTransform(Track, _Time, _Host);

    EvaluateTracks(Track.Children, _Time, _Host);
  }
}

void CAnimationPreview::ApplyTransform(const TTimelineTrack & _Track, TTimelineTicks _Time, ISceneAnimationHost & _Host)
{
  auto * Node = _Host.FindSceneNode(_Track.Target.Slot);
  if (!Node)
    return;

  const auto   Value    = EvaluateKeys(_Track.Keys, _Time);
  const auto & Property = _Track.Target.Property;

  if (Property == PositionProperty && std::holds_alternative<Ogre::Vector3>(Value))
    Node->setPosition(std::get<Ogre::Vector3>(Value));
  else
  if (Property == OrientationProperty && std::holds_alternative<Ogre::Quaternion>(Value))
  {
    auto Orientation = std::get<Ogre::Quaternion>(Value);
    Orientation.normalise();
    Node->setOrientation(Orientation);
  }
  else
  if (Property == ScaleProperty && std::holds_alternative<Ogre::Vector3>(Value))
    Node->setScale(std::get<Ogre::Vector3>(Value));
  else
    return;

  m_TouchedNodes.insert(_Track.Target.Slot);
}

} // namespace editor
