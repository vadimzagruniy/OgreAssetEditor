#include "TimelineKeyEvaluation.h"

#include <algorithm>
#include <type_traits>
#include <variant>

namespace editor
{

namespace
{

template <class TValue>
constexpr bool IsBlendable = std::is_same_v<TValue, float> || std::is_same_v<TValue, Ogre::Vector3> || std::is_same_v<TValue, Ogre::ColourValue>;

template <class TValue>
TValue Lerp(const TValue & _From, const TValue & _To, float _Alpha)
{
  return _From + (_To - _From) * _Alpha;
}

template <class TValue>
TValue CatmullRom(const TValue & _Before, const TValue & _From, const TValue & _To, const TValue & _After, float _Alpha)
{
  const float AlphaSquared = _Alpha * _Alpha;
  const float AlphaCubed   = AlphaSquared * _Alpha;

  return (_From * 2.0f +
          (_To - _Before) * _Alpha +
          (_Before * 2.0f - _From * 5.0f + _To * 4.0f - _After) * AlphaSquared +
          (_From * 3.0f - _Before - _To * 3.0f + _After) * AlphaCubed) * 0.5f;
}

template <class TValue>
TValue GetOr(const TPropertyValue & _Value, const TValue & _Fallback)
{
  return std::holds_alternative<TValue>(_Value) ? std::get<TValue>(_Value) : _Fallback;
}

TPropertyValue InterpolateSegment(const std::vector<TTimelineKey> & _Keys, std::size_t _Index, float _Alpha)
{
  const auto & From = _Keys[_Index];
  const auto & To   = _Keys[_Index + 1];

  if (From.Interpolation == ETimelineInterpolation::Step || From.Value.index() != To.Value.index())
    return From.Value;

  const bool   IsSpline = From.Interpolation == ETimelineInterpolation::Spline;
  const auto & Before   = _Keys[_Index > 0 ? _Index - 1 : _Index].Value;
  const auto & After    = _Keys[std::min(_Index + 2, _Keys.size() - 1)].Value;

  return std::visit([&](const auto & _FromValue) -> TPropertyValue
  {
    using TValue = std::decay_t<decltype(_FromValue)>;

    if constexpr (IsBlendable<TValue>)
    {
      const auto & ToValue = std::get<TValue>(To.Value);

      if (!IsSpline)
        return Lerp(_FromValue, ToValue, _Alpha);

      return CatmullRom(GetOr(Before, _FromValue), _FromValue, ToValue, GetOr(After, ToValue), _Alpha);
    }
    else if constexpr (std::is_same_v<TValue, Ogre::Quaternion>)
    {
      return Ogre::Quaternion::Slerp(_Alpha, _FromValue, std::get<TValue>(To.Value), true);
    }
    else
    {
      return _FromValue;
    }
  }, From.Value);
}

} // namespace

TPropertyValue EvaluateKeys(const std::vector<TTimelineKey> & _Keys, TTimelineTicks _Time)
{
  if (_Keys.empty())
    return {};

  if (_Time <= _Keys.front().Time)
    return _Keys.front().Value;

  if (_Time >= _Keys.back().Time)
    return _Keys.back().Value;

  const auto NextKey = std::upper_bound(_Keys.begin(), _Keys.end(), _Time, [](TTimelineTicks _Value, const TTimelineKey & _Key)
  {
    return _Value < _Key.Time;
  });

  const auto  Index = static_cast<std::size_t>(NextKey - _Keys.begin()) - 1;
  const auto  Span  = static_cast<float>(_Keys[Index + 1].Time - _Keys[Index].Time);
  const float Alpha = static_cast<float>(_Time - _Keys[Index].Time) / Span;

  return InterpolateSegment(_Keys, Index, Alpha);
}

} // namespace editor
