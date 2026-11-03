#pragma once

#include <memory>
#include <vector>

namespace editor
{

class CSceneDocument;
class ISceneDropHandler;
struct TSceneDrop;

class CSceneDropHandlers
{
public: // Interface

  void Register(std::shared_ptr<ISceneDropHandler> _Handler);

  std::shared_ptr<ISceneDropHandler> FindHandler(const TSceneDrop & _Drop, const CSceneDocument & _Document) const;

private: // Attributes

  std::vector<std::shared_ptr<ISceneDropHandler>> m_Handlers;
};

} // namespace editor
