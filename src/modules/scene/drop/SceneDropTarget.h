#pragma once

#include "modules/scene/drop/OutlinerDropTarget.h"
#include "modules/scene/drop/ViewportDropTarget.h"

#include <variant>

namespace editor
{

using TSceneDropTarget = std::variant<TViewportDropTarget, TOutlinerDropTarget>;

} // namespace editor
