#include "AnimationTrackSource.h"
#include "AnimationSlots.h"
#include "AnimationTracks.h"
#include "editor/assets/AssetReference.h"
#include "editor/inspector/PropertySchema.h"
#include "editor/timeline/model/Timeline.h"
#include "modules/scene/animation/ISceneAnimationHost.h"
#include "modules/scene/model/SceneGraph.h"
#include "modules/scene/model/SceneNode.h"
#include "modules/scene/model/SceneNodeTypes.h"

#include <array>
#include <cstdint>
#include <string>
#include <utility>

namespace editor
{

namespace
{

constexpr std::uint32_t DefaultTrackColor = 0xFFB4B4B4;

constexpr std::array<std::uint32_t, 8> TrackPalette
{
  0xFF4A5AE8,
  0xFF5CC45A,
  0xFFE8A04A,
  0xFF3CB4F0,
  0xFFD070B8,
  0xFFC8C83C,
  0xFF8C64F0,
  0xFF50D2A0
};

bool FindAnimatableIndex(const std::vector<TPropertyDesc> & _Properties, const std::string & _Identifier, std::size_t & _Index)
{
  for (const auto & Property : _Properties)
  {
    if (Property.Type == EPropertyType::Group)
    {
      if (FindAnimatableIndex(Property.Children, _Identifier, _Index))
        return true;

      continue;
    }

    if (!Property.Meta.IsAnimatable)
      continue;

    if (Property.Identifier == _Identifier)
      return true;

    ++_Index;
  }

  return false;
}

bool HasTrack(const std::vector<TTimelineTrack> & _Tracks, const TTimelineTarget & _Target)
{
  for (const auto & Track : _Tracks)
  {
    if (Track.Target == _Target || HasTrack(Track.Children, _Target))
      return true;
  }

  return false;
}

void CollectTemplates(const std::vector<TPropertyDesc> & _Properties, const std::string & _GroupLabel, const std::string & _Slot, const CTimeline & _Timeline, std::vector<TTimelineTrackTemplate> & _Templates)
{
  for (const auto & Property : _Properties)
  {
    if (Property.Type == EPropertyType::Group)
    {
      CollectTemplates(Property.Children, Property.Label, _Slot, _Timeline, _Templates);
      continue;
    }

    if (!Property.Meta.IsAnimatable)
      continue;

    TTimelineTrack Track;
    Track.TypeId = NodePropertyTrackType;
    Track.Name   = Property.Label;
    Track.Target = {_Slot, Property.Identifier};

    if (HasTrack(_Timeline.GetTracks(), Track.Target))
      continue;

    const auto Label = _GroupLabel.empty() ? Property.Label : _GroupLabel + " / " + Property.Label;
    _Templates.push_back({Label, std::move(Track)});
  }
}

std::vector<TTimelineTrackTemplate> MakeNodeTemplates(const CSceneGraph & _Graph, const std::string & _NodeName, const std::string & _Slot, const CTimeline & _Timeline)
{
  if (_NodeName.empty() || !_Graph.HasNode(_NodeName))
    return {};

  const auto & Schema = _Graph.GetNodeTypes().GetSchema(std::string(_Graph.GetNode(_NodeName).GetTypeId()));

  std::vector<TTimelineTrackTemplate> Templates;
  CollectTemplates(Schema.Properties, {}, _Slot, _Timeline, Templates);

  return Templates;
}

} // namespace

//
// Constructor
//

CAnimationTrackSource::CAnimationTrackSource(std::shared_ptr<const CAnimationSlots> _Slots):
  m_Slots(std::move(_Slots))
{
  // Empty
}

//
// ITimelineTrackSource
//

std::vector<TTimelineTrackTemplate> CAnimationTrackSource::GetTrackTemplates(const CTimeline & _Timeline) const
{
  const auto Host = m_Slots->LockHost();
  if (!Host)
    return {};

  const auto NodeName = Host->GetSelectedNode();
  const auto Slot     = Host->FindSlot(m_Slots->GetAnimationId(), NodeName);

  return MakeNodeTemplates(Host->GetGraph(), NodeName, Slot, _Timeline);
}

std::vector<TTimelineTrackTemplate> CAnimationTrackSource::GetTargetTemplates(const CTimeline & _Timeline, const std::string & _Slot) const
{
  const auto Host = m_Slots->LockHost();
  if (!Host || _Slot.empty())
    return {};

  return MakeNodeTemplates(Host->GetGraph(), m_Slots->ResolveSlot(_Slot), _Slot, _Timeline);
}

std::string CAnimationTrackSource::GetTrackLabel(const TTimelineTrack & _Track) const
{
  if (_Track.TypeId != TimelineGroupType || _Track.Target.Slot.empty())
    return _Track.Name;

  return m_Slots->ResolveSlot(_Track.Target.Slot);
}

TPropertyValue CAnimationTrackSource::CaptureValue(const TTimelineTrack & _Track) const
{
  const auto Host = m_Slots->LockHost();
  if (!Host || _Track.TypeId != NodePropertyTrackType)
    return {};

  const auto & Graph    = Host->GetGraph();
  const auto   NodeName = m_Slots->ResolveSlot(_Track.Target.Slot);

  if (!Graph.HasNode(NodeName))
    return {};

  return Graph.GetNode(NodeName).GetValue(_Track.Target.Property);
}

bool CAnimationTrackSource::IsTargetValid(const TTimelineTrack & _Track) const
{
  const auto Host = m_Slots->LockHost();
  if (!Host || _Track.Target.Slot.empty())
    return true;

  return Host->GetGraph().HasNode(m_Slots->ResolveSlot(_Track.Target.Slot));
}

bool CAnimationTrackSource::CanBindToSelection(const TTimelineTrack & _Track) const
{
  const auto Host = m_Slots->LockHost();
  if (!Host || _Track.Target.Slot.empty())
    return false;

  const auto NodeName = Host->GetSelectedNode();
  return !NodeName.empty() && m_Slots->ResolveSlot(_Track.Target.Slot) != NodeName;
}

void CAnimationTrackSource::BindToSelection(const TTimelineTrack & _Track) const
{
  if (!CanBindToSelection(_Track))
    return;

  AddAnimationToScene();

  const auto Host = m_Slots->LockHost();
  Host->BindSlot(m_Slots->GetAnimationId(), _Track.Target.Slot, Host->GetSelectedNode());
}

std::uint32_t CAnimationTrackSource::GetTrackColor(const TTimelineTrack & _Track) const
{
  const auto Host = m_Slots->LockHost();
  if (!Host || _Track.TypeId != NodePropertyTrackType)
    return DefaultTrackColor;

  const auto & Graph    = Host->GetGraph();
  const auto   NodeName = m_Slots->ResolveSlot(_Track.Target.Slot);

  if (!Graph.HasNode(NodeName))
    return DefaultTrackColor;

  const auto & Schema = Graph.GetNodeTypes().GetSchema(std::string(Graph.GetNode(NodeName).GetTypeId()));

  std::size_t PropertyIndex = 0;
  if (!FindAnimatableIndex(Schema.Properties, _Track.Target.Property, PropertyIndex))
    return DefaultTrackColor;

  return TrackPalette[PropertyIndex % TrackPalette.size()];
}

bool CAnimationTrackSource::HasTrackTarget() const
{
  const auto Host = m_Slots->LockHost();
  if (!Host)
    return false;

  const auto NodeName = Host->GetSelectedNode();
  return !NodeName.empty() && Host->GetGraph().HasNode(NodeName);
}

void CAnimationTrackSource::SelectTrackTarget(const std::string & _Target) const
{
  if (const auto Host = m_Slots->LockHost())
    Host->SelectNode(_Target);
}

void CAnimationTrackSource::ShowTrackInScene(const TTimelineTrack & _Track) const
{
  const auto Host = m_Slots->LockHost();
  if (!Host || _Track.Target.Slot.empty())
    return;

  const auto NodeName = m_Slots->ResolveSlot(_Track.Target.Slot);

  if (Host->GetGraph().HasNode(NodeName) && Host->GetSelectedNode() != NodeName)
    Host->SelectNode(NodeName);
}

//
// Events
//

void CAnimationTrackSource::OnTrackAdded(const TTimelineTrack & _Track) const
{
  if (_Track.TypeId == NodePropertyTrackType)
    AddAnimationToScene();
}

//
// Implementation
//

void CAnimationTrackSource::AddAnimationToScene() const
{
  const auto Host = m_Slots->LockHost();

  if (Host && !Host->HasAnimation(m_Slots->GetAnimationId()))
    Host->AddAnimation(m_Slots->MakeAnimationRef());
}

} // namespace editor
