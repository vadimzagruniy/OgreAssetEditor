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

  CSceneAnimationHost(std::shared_ptr<const CSceneDocument> _Document, std::shared_ptr<const CSceneSelection> _Selection, CSceneSync & _Sync);

public: // ISceneAnimationHost

  const CSceneGraph & GetGraph() const override;

  std::string GetSelectedNode() const override;

  void PreviewValue(const std::string & _Name, const std::string & _Identifier, const TPropertyValue & _Value) override;

  void RestoreNode(const std::string & _Name) override;

public: // Interface

  void ApplyPreview();

private: // Types

  using TPreviewValues = std::unordered_map<std::string, TPropertyValue>;

private: // Attributes

  std::shared_ptr<const CSceneDocument>           m_Document;
  std::shared_ptr<const CSceneSelection>          m_Selection;
  CSceneSync &                                    m_Sync;
  std::unordered_map<std::string, TPreviewValues> m_PreviewValues;
};

} // namespace editor
