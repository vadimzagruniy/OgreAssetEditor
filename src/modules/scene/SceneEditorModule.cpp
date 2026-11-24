#include "SceneEditorModule.h"
#include "SceneAssetType.h"
#include "SceneFactory.h"
#include "core/ActionQueue.h"
#include "core/EditorService.h"
#include "core/Log.h"
#include "editor/assets/AssetFactoryRegistry.h"
#include "editor/assets/AssetTypeRegistry.h"
#include "editor/modules/ModuleInfo.h"
#include "editor/modules/ModuleRegistry.h"
#include "drop/MeshDropHandler.h"
#include "drop/SceneDropHandlers.h"
#include "model/SceneNode.h"
#include "model/SceneNodeTemplates.h"
#include "model/SceneNodeTypes.h"
#include "model/nodes/LightNode.h"
#include "model/nodes/MeshNode.h"
#include "model/nodes/PrimitiveNode.h"
#include "presenter/SceneEditorController.h"
#include "render/LightNodeBinding.h"
#include "render/MeshNodeBinding.h"
#include "render/PrimitiveNodeBinding.h"
#include "render/SceneNodeBindings.h"
#include "ui/ToolBoxDesc.h"
#include "ui/WindowManager.h"
#include "views/SceneObjectsWindow.h"
#include "views/SceneOutlinerWindow.h"

#include <array>
#include <string>
#include <utility>

namespace editor
{

namespace
{

void RegisterTemplates(CSceneNodeTemplates & _Templates)
{
  _Templates.Register("node.empty", "Empty", "General", CSceneNode());

  const std::array<std::pair<const char *, EPrimitiveShape>, 3> Primitives{{
    {"Cube",   EPrimitiveShape::Cube  },
    {"Sphere", EPrimitiveShape::Sphere},
    {"Plane",  EPrimitiveShape::Plane }
  }};

  for (const auto & [Label, Shape] : Primitives)
  {
    CPrimitiveNode Primitive;
    Primitive.SetShape(Shape);

    _Templates.Register(std::string("primitive.") + Label, Label, "Geometry", Primitive);
  }

  const std::array<std::pair<const char *, ELightType>, 3> Lights{{
    {"Point Light",       ELightType::Point      },
    {"Directional Light", ELightType::Directional},
    {"Spot Light",        ELightType::Spot       }
  }};

  for (const auto & [Label, LightType] : Lights)
  {
    CLightNode Light;
    Light.SetLightType(LightType);

    _Templates.Register(std::string("light.") + Label, Label, "Lights", Light);
  }
}

} // namespace

//
// IModule
//

const TModuleInfo & CSceneEditorModule::GetInfo() const noexcept
{
  static const TModuleInfo Info{"SceneEditor", {}};
  return Info;
}

bool CSceneEditorModule::Startup(core::CEditorService & _Services, std::string & _Error)
{
  auto AssetTypes     = _Services.GetService<CAssetTypeRegistry>();
  auto AssetFactories = _Services.GetService<CAssetFactoryRegistry>();
  auto WindowManager  = _Services.GetService<ui::CWindowManager>();
  auto ActionQueue    = _Services.GetService<core::CActionQueue>();
  auto EditorLog      = _Services.GetService<core::CLog>();

  if (!AssetTypes || !AssetFactories || !WindowManager || !ActionQueue || !EditorLog)
  {
    _Error = "Scene Editor module requires the asset type registry, asset factory registry, window manager, action queue and log services.";
    return false;
  }

  m_WindowManager = std::move(WindowManager);

  auto NodeTypes = std::make_shared<CSceneNodeTypes>();
  NodeTypes->Register<CSceneNode>("Empty", "General");
  NodeTypes->Register<CMeshNode>("Mesh", "Geometry");
  NodeTypes->Register<CPrimitiveNode>("Primitive", "Geometry");
  NodeTypes->Register<CLightNode>("Light", "Lights");

  auto Templates = std::make_shared<CSceneNodeTemplates>();
  RegisterTemplates(*Templates);

  auto NodeBindings = std::make_shared<CSceneNodeBindings>();
  NodeBindings->Register<CMeshNodeBinding>(CMeshNode::TypeId);
  NodeBindings->Register<CPrimitiveNodeBinding>(CPrimitiveNode::TypeId);
  NodeBindings->Register<CLightNodeBinding>(CLightNode::TypeId);

  auto DropHandlers = std::make_shared<CSceneDropHandlers>();
  DropHandlers->Register(std::make_shared<CMeshDropHandler>());

  auto Controller = std::make_shared<CSceneEditorController>(std::move(ActionQueue), AssetTypes, std::move(DropHandlers), std::move(Templates));

  const ui::TToolBoxDesc OutlinerDesc{GetInfo().Name, "Scene Outliner", false, ui::EDockSlot::LeftBottom, 0};
  m_WindowManager->RegisterToolBox(OutlinerDesc, std::make_shared<CSceneOutlinerWindow>(OutlinerDesc, Controller));

  const ui::TToolBoxDesc ObjectsDesc{GetInfo().Name, "Scene Objects", false, ui::EDockSlot::RightBottom, 0};
  m_WindowManager->RegisterToolBox(ObjectsDesc, std::make_shared<CSceneObjectsWindow>(ObjectsDesc, Controller));

  AssetTypes->Register(std::make_shared<CSceneAssetType>(std::move(Controller), NodeTypes, std::move(NodeBindings), std::move(EditorLog)));
  AssetFactories->Register(std::make_shared<CSceneFactory>(std::move(NodeTypes)));

  return true;
}

void CSceneEditorModule::Shutdown() noexcept
{
  if (m_WindowManager)
    m_WindowManager->UnregisterModuleToolBoxes(GetInfo().Name);
}

IMPLEMENT_MODULE(CSceneEditorModule, "SceneEditor")

} // namespace editor
