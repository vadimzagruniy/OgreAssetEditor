#pragma once

#include <OgrePrerequisites.h>

#include <optional>
#include <string>

struct ImVec2;

namespace editor
{

class CRenderScene;
class CSceneGraph;
struct TViewportFrame;

class CSceneViewportIcons final
{
public: // Constructor

  explicit CSceneViewportIcons(const CRenderScene & _RenderScene);

public: // Interface

  std::string Draw(const TViewportFrame & _Frame, const CSceneGraph & _Graph, const std::string & _Selection) const;

private: // Implementation

  std::string FindHovered(const TViewportFrame & _Frame, const CSceneGraph & _Graph) const;

  std::optional<ImVec2> FindScreenPosition(const TViewportFrame & _Frame, const CSceneGraph & _Graph, const std::string & _Name) const;

  std::optional<ImVec2> Project(const TViewportFrame & _Frame, const Ogre::Vector3 & _Point) const;

private: // Attributes

  const CRenderScene & m_RenderScene;
};

} // namespace editor
