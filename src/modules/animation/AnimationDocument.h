#pragma once

#include "editor/timeline/TimelineDocument.h"

#include <string>
#include <vector>

namespace editor
{

class CAnimationDocument final:
  public CTimelineDocument
{
public: // Constructor

  explicit CAnimationDocument(TAssetRecord _Record);

public: // CAssetDocument

  std::vector<TAssetProperty> GetProperties() const override;

protected: // CAssetDocument

  bool OnSave(std::string & _Error) override;

public: // Interface

  bool Load(const std::string & _Text, std::string & _Error);
};

} // namespace editor
