# Ogre Asset Editor

A modular asset editor for [OGRE 3D](https://www.ogre3d.org/) built with [Dear ImGui](https://github.com/ocornut/imgui) (docking branch).

The editor shell knows nothing about concrete asset types. Each asset type lives in its own module that registers
its asset type, editor workspace and tool windows at startup, so a new asset editor is added without touching the
editor core.

> Work in progress — a personal C++ project.

## Features

- Module system with declared dependencies: modules are started in dependency order, cycles are detected,
  and a failed startup rolls back the modules already started.
- Asset type registry: file extension → asset type → editor workspace.
- Tabbed editor workspaces with a single instance per file.
- OGRE resources are released when their editor tab is closed (unless still used elsewhere).
- Built-in modules: Content Browser, Properties, Console, Texture viewer, Mesh info, Text editor.

## Building

Requirements: Windows, Visual Studio 2022, CMake 3.21+.

```powershell
git clone --recursive https://github.com/vadimzagruniy/OgreAssetEditor.git
cd OgreAssetEditor
cmake --preset vs2022-x64
cmake --build --preset vs2022-debug --parallel
```

Already cloned without `--recursive`? Run `git submodule update --init`.

The first configure builds OGRE and its dependencies (SDL2, pugixml), which takes a while.
The solution is generated at `build/vs2022-x64/OgreAssetEditor.sln`.

Run `OgreResourceEditor.exe` with a project directory as an argument; without one it opens `sample_project`.
A project is any directory that contains an `Assets` folder.

## Architecture

```
src/
  core/        type keys, service locator, logging, signals, commands, undo stack
  platform/    OGRE + SDL application, frame and input dispatch
  ui/          ImGui window base classes and the tool window manager
  editor/
    assets/      project asset scanning, asset type interface and registry
    modules/     module interface, static module registry, module lifecycle
    workspaces/  asset instances, editor workspaces and the workspace (tab) manager
  modules/     one folder per module
    console/  content_browser/  properties/  mesh/  texture/  text/
third_party/   OGRE 14.5.2 and Dear ImGui 1.91.1-docking as git submodules
```

Modules talk to the editor only through services (`CEditorService`): asset manager, asset type registry,
workspace manager, window manager and log. A typical module:

1. declares its name and dependencies in `GetInfo()`;
2. in `Startup()` takes the services it needs and registers its asset type and/or tool windows;
3. registers itself with `IMPLEMENT_MODULE(CMyModule, "MyModule")`.

## Code conventions

- One class per file; `.h` for headers, `.cpp` for sources.
- Function bodies live in `.cpp` files (template code is the only exception).
- Headers forward-declare dependencies; includes go into `.cpp` wherever possible.
- Includes are rooted at `src/` (`#include "editor/assets/AssetType.h"`).
- Prefixes: `C` class, `I` interface, `T` struct/alias, `E` enum, `m_` member, `_` parameter.
- Empty function bodies contain `// Empty`.

## License

MIT — see [LICENSE](LICENSE). OGRE and Dear ImGui are distributed under their own MIT licenses.
