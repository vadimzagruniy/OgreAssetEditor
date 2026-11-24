#include "SceneTemplatePayload.h"
#include "ui/DragPayload.h"

namespace editor
{

const ui::CDragPayload & GetTemplatePayload()
{
  static const ui::CDragPayload Payload("SCENE_TEMPLATE");
  return Payload;
}

} // namespace editor
