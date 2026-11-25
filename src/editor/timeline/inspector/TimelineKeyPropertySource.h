#pragma once

#include "editor/inspector/IPropertySource.h"
#include "editor/timeline/model/TimelineKey.h"

#include <memory>
#include <optional>

namespace editor
{

class CTimelineDocument;
class CTimelineSelection;

class CTimelineKeyPropertySource final:
  public IPropertySource
{
public: // Constructor

  CTimelineKeyPropertySource(std::shared_ptr<CTimelineDocument> _Document, std::shared_ptr<const CTimelineSelection> _Selection);

public: // IPropertySource

  std::string GetTitle() const override;

  const TPropertySchema & GetSchema() const override;

  TPropertyValue GetValue(const std::string & _Identifier) const override;

  void Preview(const std::string & _Identifier, const TPropertyValue & _Value) override;

  void Commit() override;

public: // Interface

  bool HasKey() const;

private: // Implementation

  std::optional<TTimelineKey> FindSelectedKey() const;

private: // Attributes

  std::shared_ptr<CTimelineDocument>        m_Document;
  std::shared_ptr<const CTimelineSelection> m_Selection;
};

} // namespace editor
