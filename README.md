<img width="1919" height="1046" alt="image" src="https://github.com/user-attachments/assets/c1d0af06-5dba-4c21-b0d7-db12e32f7b79" />

# Ogre Asset Editor

A modular asset editor for [OGRE 3D](https://www.ogre3d.org/) built with [Dear ImGui](https://github.com/ocornut/imgui) (docking branch).

The editor shell knows nothing about concrete asset types. Each asset type lives in its own module that registers
its asset type, editor workspace and tool windows at startup, so a new asset editor is added without touching the
editor core.

> Work in progress — a personal C++ project.

## Features

### Editor shell

- Module system with declared dependencies: modules are started in dependency order, cycles are detected,
  and a failed startup rolls back the modules already started.
- Asset type registry (file extension → asset type → editor workspace) and asset factories: a module registers
  a factory, and the Content Browser offers it in its Create menu.
- Tabbed editor workspaces with a single instance per file. A tab declares its own tool windows, which the editor
  shows only while that tab is active.
- Default dock layout: the editor defines the panel proportions, each module picks a slot for its tool windows.
- OGRE resources are released when their editor tab is closed (unless still used elsewhere).
- Undo/redo per tab: every edit is a command on the tab's undo stack, a slider or gizmo drag is one command,
  Ctrl+Z / Ctrl+Y, Ctrl+S, `*` on modified tabs and save prompts when closing a tab or the editor.

### Viewport

- Shared by the scene editor and the mesh and material previews: the scene is rendered into a texture and shown
  inside the tab.
- Camera: right mouse or Alt + left mouse — orbit, Alt + right mouse — dolly, middle mouse — pan, wheel — zoom,
  right mouse + WASD/QE — fly, F — frame the content. Clip range and zoom limits follow the scene size.
- Ground grid and a view axis gizmo that aligns the camera to an axis on click.

### Scene editor

- `.oscene` scenes: a node hierarchy with transforms and meshes, stored as JSON. New scenes are created from
  the Content Browser.
- Scene Outliner: create, rename (F2), delete (Del) and reparent nodes by drag and drop.
- Picking in the viewport and a transform gizmo: move / rotate / scale (W / E / R) in world or local space.
- Meshes are dropped from the Content Browser onto the viewport or the outliner. Drops go through drop handlers
  registered by the module, so new asset kinds are added without touching the views.

### Properties inspector

- One inspector window for every asset editor. The active tab hands it a property source; the source describes its
  properties with a schema (groups, types, ranges, options), and the window builds the controls from that schema.
- Edits go back to the source as preview and commit, so a drag stays one undo command.
- Controls: bool, float, enum, colour, vector, rotation (Euler angles), text and asset reference.
- Asset reference field: a path inside the project, typed by hand or picked in a file dialog limited to the `Assets`
  folder and the asset type's extensions, dropped from the Content Browser, or opened in its editor. Missing assets
  are highlighted.

### Asset editors

- Content browser built with MVP: a folder tree, a navigation bar and list/grid content views are passive views that
  talk to the controller only through `IContentBrowserPresenter`; user actions are queued and applied after all views
  are drawn. Create assets and folders, rename (F2), duplicate (Ctrl+D), delete (Del) with confirmation, filter,
  breadcrumbs, drag assets into other windows. Open tabs follow file operations: renamed assets keep their tabs,
  deleted assets close them.
- Material editor for `.material` scripts: edits run on working copies of the OGRE materials, so the preview updates
  live while the originals stay untouched until the file is saved; closing the tab without saving discards the copies.
  Material Outline selects a pass or a texture unit, Properties edits pass colours, shininess, blending, depth write,
  culling, texture and addressing. Preview on a sphere, cube or plane.
- Mesh viewer, texture viewer, text editor and console.

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
The sample project includes `ogrehead.mesh` with its material and textures from the OGRE samples and a test scene.

## Architecture

```
src/
  core/        type keys, service locator, logging, signals, commands, undo stack, action queue
  platform/    OGRE + SDL application, Windows shell and file dialog
  ui/          window base classes, tool window manager, modal windows, popup menu, drag and drop payloads
    elements/    retained UI elements (buttons, combo boxes, drags, inputs) with fluent setup
    layout/      row/column layouts and child areas
    dock_layout/ default dock layout: dock slots and the layout builder
    widgets/     small reusable ImGui widgets (splitter)
  editor/
    assets/      project asset scanning, asset types, asset factories and their registries
    inspector/   property schema, property values and the property source interface
    modules/     module interface, static module registry, module lifecycle
    render/      offscreen render scene and the preview scene used by viewers
    viewport/    viewport, camera and navigation, grid, view axis gizmo
      gizmo/       transform gizmo (move, rotate, scale)
    workspaces/  asset instances, editor workspaces and the workspace (tab) manager
  modules/     one folder per module
    console/  content_browser/  properties/  mesh/  material/  scene/  texture/  text/
    content_browser/
      presenter/   IContentBrowserPresenter, CContentBrowserController (no ImGui)
      views/       folder tree, navigation bar, list and grid views, toolbar, delete confirmation
    properties/
      controls/    property controls built from a schema, asset reference field
    material/
      MaterialInstance   original materials and their working copies, saving
      MaterialDocument   reads and edits pass / texture unit state, turns edits into undo commands
      inspector/         property sources for a pass and a texture unit
      views/             outline tool window
    scene/
      model/             scene nodes, transforms, subtrees
      SceneDocument      selection, node edits and previews as undo commands
      render/            syncs the scene model to OGRE scene nodes, picking
      drop/              drop handlers for assets dropped onto the scene
      inspector/         property source for a scene node
      presenter/ views/  scene editor controller and the outliner
third_party/   OGRE 14.5.2 and Dear ImGui 1.91.1-docking as git submodules
```

Modules talk to the editor only through services (`CEditorService`): asset manager, asset type and asset factory
registries, workspace manager, window manager, action queue and log. A typical module:

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
