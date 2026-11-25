#pragma once

#include "TimelineTrackTypeInfo.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace editor
{

class ITimelineTrackEvaluator;

class CTimelineTrackTypes final
{
public: // Constructor

  CTimelineTrackTypes();

public: // Interface

  void Register(TTimelineTrackTypeInfo _Info, std::shared_ptr<ITimelineTrackEvaluator> _Evaluator);

  ETimelineItemKind GetItemKind(const std::string & _TypeId) const;

  std::shared_ptr<ITimelineTrackEvaluator> FindEvaluator(const std::string & _TypeId) const;

  const std::vector<std::shared_ptr<ITimelineTrackEvaluator>> & GetEvaluators() const noexcept;

private: // Types

  struct TEntry
  {
    TTimelineTrackTypeInfo                   Info;
    std::shared_ptr<ITimelineTrackEvaluator> Evaluator;
  };

private: // Attributes

  std::unordered_map<std::string, TEntry>               m_Types;
  std::vector<std::shared_ptr<ITimelineTrackEvaluator>> m_Evaluators;
};

} // namespace editor
