# Ogre Resource Editor

A standalone portfolio pet project using OGRE 14.5.2 and Dear ImGui 1.91.1 docking sources copied from the dependency path provided for this project. The editor is structured around modules and service registration so an asset editor can be added without changing the editor shell.

## Build with Visual Studio 2022

Open a Developer PowerShell in this directory and run:

```powershell
cmake --preset vs2022-x64
cmake --build --preset vs2022-debug --parallel
```

CMake generates `build/vs2022-x64/OgreAssetEditor.sln`. The first configure/build may fetch and compile OGRE's SDL2 and pugixml dependencies. The sample project is `sample_project`; launch `OgreResourceEditor.exe` with a project directory argument to open another project. A project contains an `Assets` folder.

The project builds OgreMain, OgreBites, OgreOverlay with the copied docking-enabled ImGui, and available GL3+ / D3D11 render systems. OGRE only enables a render system when its platform SDK is detected.

## Layout

```
src/
  core/        type keys, service locator, logging, signals, commands, undo stack
  platform/    OGRE + SDL application, frame and input dispatch
  ui/          ImGui window base classes and the toolbox window manager
  editor/
    assets/      project asset scanning, asset type interface and registry
    modules/     module interface, static module registry, module lifecycle
    workspaces/  asset instances, editor workspaces and the workspace (tab) manager
  modules/     one folder per module: Module + its AssetType / Instance / Workspace / ToolBox windows
    console/  content_browser/  properties/  mesh/  texture/  text/
third_party/
  ogre/        OGRE 14.5.2 source
  imgui/       Dear ImGui docking sources
```

## Code conventions

- One class per file; `.h` for headers, `.cpp` for sources.
- Function bodies live in `.cpp` files (template code is the only exception).
- Headers forward-declare dependencies; includes go into `.cpp` wherever possible.
- Includes are rooted at `src/` (`#include "editor/assets/AssetType.h"`).
- Prefixes: `C` class, `I` interface, `T` struct/alias, `E` enum, `m_` member, `_` parameter.
- Empty function bodies contain `// Empty`.
