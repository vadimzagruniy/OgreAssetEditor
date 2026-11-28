#pragma once

#include "ISceneAnimationHost.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace editor
{

class CSceneDocument;
class CSceneSelection;
class CSceneSync;

class CSceneAnimationHost final:
  public ISceneAnimationHost
{
public: // Constructor

  CSceneAnimationHost(std::shared_ptr<CSceneDocument> _Document, std::shared_ptr<CSceneSelection> _Selection, CSceneSync & _Sync);

public: // ISceneAnimationHost

  const CSceneGraph & GetGraph() const override;

  std::string GetSelectedNode() const override;

  void SelectNode(const std::string & _Name) override;

  void PreviewValue(const std::string & _Name, const std::string & _Identifier, const TPropertyValue & _Value) override;

  void RestoreNode(const std::string & _Name) override;

  std::vector<TAssetReference> GetAnimationRefs() const override;

  bool HasAnimation(const TAssetId & _AnimationId) const override;

  void AddAnimation(const TAssetReference & _AnimationRef) override;

  void RemoveAnimation(const TAssetReference & _AnimationRef) override;

  std::string ResolveSlot(const TAssetId & _AnimationId, const std::string & _Slot) const override;

  std::string FindSlot(const TAssetId & _AnimationId, const std::string & _NodeName) const override;

  void BindSlot(const TAssetId & _AnimationId, const std::string & _Slot, const std::string & _NodeName) override;

public: // Interface

  void ApplyPreview();

private: // Types

  using TPreviewValues = std::unordered_map<std::string, TPropertyValue>;

private: // Attributes

  std::shared_ptr<CSceneDocument>                 m_Document;
  std::shared_ptr<CSceneSelection>                m_Selection;
  CSceneSync &                                    m_Sync;
  std::unordered_map<std::string, TPreviewValues> m_PreviewValues;
};

} // namespace editor
