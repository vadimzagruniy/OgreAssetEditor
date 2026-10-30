#pragma once

#include "editor/workspaces/AssetInstance.h"

#include <OgrePrerequisites.h>
#include <OgreSharedPtr.h>

#include <cstddef>
#include <string>
#include <vector>

namespace editor
{

class CMaterialInstance final:
  public CAssetInstance
{
public: // Constructor/Destructor

  CMaterialInstance(TAssetRecord _Record, const std::vector<Ogre::MaterialPtr> & _Materials, bool _HasExpandedFeatures);

  ~CMaterialInstance() override;

  CMaterialInstance(const CMaterialInstance &) = delete;

  CMaterialInstance & operator=(const CMaterialInstance &) = delete;

public: // Interface

  std::size_t GetMaterialCount() const noexcept;

  const Ogre::MaterialPtr & GetWorkingCopy(std::size_t _Index) const;

  const std::string & GetMaterialName(std::size_t _Index) const;

  bool HasExpandedFeatures() const noexcept;

  bool Save(std::string & _Error);

public: // CAssetInstance

  std::vector<TAssetProperty> GetProperties() const override;

private: // Types

  struct TMaterialCopy
  {
    Ogre::MaterialPtr Original;
    Ogre::MaterialPtr WorkingCopy;
  };

private: // Implementation

  static void ApplyWorkingCopy(TMaterialCopy & _Material);

private: // Attributes

  std::vector<TMaterialCopy> m_Materials;
  bool                       m_HasExpandedFeatures;
};

} // namespace editor
