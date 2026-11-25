#pragma once

#include <memory>
#include <string>

namespace editor
{

class CAssetDocument;
class ISceneAnimationHost;

class CAnimationSlots final
{
public: // Constructor

  CAnimationSlots(std::weak_ptr<ISceneAnimationHost> _Host, std::shared_ptr<const CAssetDocument> _Animation);

public: // Interface

  std::shared_ptr<ISceneAnimationHost> LockHost() const;

  std::string GetAnimationPath() const;

  std::string ResolveSlot(const std::string & _Slot) const;

private: // Attributes

  std::weak_ptr<ISceneAnimationHost>    m_Host;
  std::shared_ptr<const CAssetDocument> m_Animation;
};

} // namespace editor
