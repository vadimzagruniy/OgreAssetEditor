#include "AnimationFactory.h"
#include "AnimationSerializer.h"
#include "editor/assets/AssetFactoryInfo.h"
#include "editor/timeline/model/Timeline.h"

namespace editor
{

//
// IAssetFactory
//

const TAssetFactoryInfo & CAnimationFactory::GetInfo() const noexcept
{
  static const TAssetFactoryInfo Info{"animation.empty", "animation", "Animation", "New Animation"};
  return Info;
}

bool CAnimationFactory::CreateContent(std::string & _Content, std::string & /*_Error*/) const
{
  _Content = WriteAnimation(CTimeline());

  return true;
}

} // namespace editor
