#pragma once

#include "editor/documents/AssetDocument.h"

#include <OgrePrerequisites.h>
#include <OgreSharedPtr.h>

#include <cstddef>
#include <string>
#include <vector>

namespace editor
{

class CMaterialInstance final:
  public CAssetDocument
{
public: // Constructor/Destructor

  CMaterialInstance(TAssetRecord _Record, const std::vector<Ogre::MaterialPtr> & _Materials, bool _HasExpandedFeatures);

  ~CMaterialInstance() override;

  CMaterialInstance(const CMaterialInstance &) = delete;

  CMaterialInstance & operator=(const CMaterialInstance &) = delete;

public: // CAssetDocument

  std::vector<TAssetProperty> GetProperties() const override;

protected: // CAssetDocument

  bool OnSave(std::string & _Error) override;

public: // Interface

  std::size_t GetMaterialCount() const noexcept;

  const Ogre::MaterialPtr & GetWorkingCopy(std::size_t _Index) const;

  const std::string & GetMaterialName(std::size_t _Index) const;

  bool HasExpandedFeatures() const noexcept;

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
