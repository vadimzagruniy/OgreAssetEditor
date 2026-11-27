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

  std::vector<std::string> GetAnimations() const override;

  bool HasAnimation(const std::string & _Path) const override;

  void AddAnimation(const std::string & _Path) override;

  void RemoveAnimation(const std::string & _Path) override;

  std::string ResolveSlot(const std::string & _Path, const std::string & _Slot) const override;

  std::string FindSlot(const std::string & _Path, const std::string & _NodeName) const override;

  void BindSlot(const std::string & _Path, const std::string & _Slot, const std::string & _NodeName) override;

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
