#pragma once

#include "editor/inspector/IPropertySource.h"

#include <memory>
#include <string>

namespace editor
{

class CSceneDocument;
struct TSceneNodeProperties;

class CSceneNodePropertySource final:
  public IPropertySource
{
public: // Constructor

  explicit CSceneNodePropertySource(std::shared_ptr<CSceneDocument> _Document);

public: // IPropertySource

  std::string GetTitle() const override;

  const TPropertySchema & GetSchema() const override;

  TPropertyValue GetValue(const std::string & _Identifier) const override;

  void Preview(const std::string & _Identifier, const TPropertyValue & _Value) override;

  void Commit() override;

private: // Implementation

  void PreviewMesh(const std::string & _Name, TSceneNodeProperties & _Properties, const TPropertyValue & _Value);

private: // Attributes

  std::shared_ptr<CSceneDocument> m_Document;
};

} // namespace editor
