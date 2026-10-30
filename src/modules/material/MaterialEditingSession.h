#pragma once

#include <memory>

namespace editor
{

class CMaterialDocument;

class CMaterialEditingSession
{
public: // Interface

  void SetDocument(const std::shared_ptr<CMaterialDocument> & _Document);

  void ResetDocument(const CMaterialDocument & _Document);

  std::shared_ptr<CMaterialDocument> GetDocument() const;

private: // Attributes

  std::weak_ptr<CMaterialDocument> m_Document;
};

} // namespace editor
