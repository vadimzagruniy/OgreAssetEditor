#pragma once

#include "editor/assets/AssetReference.h"
#include "modules/scene/model/SceneNode.h"

#include <string>
#include <string_view>
#include <vector>

namespace editor
{

class CMeshNode final:
  public CSceneNode
{
public: // Constants

  static constexpr std::string_view TypeId = "mesh";

public: // CSceneNode

  std::string_view GetTypeId() const noexcept override;

  void DescribeProperties(std::vector<TPropertyDesc> & _Properties) const override;

  TPropertyValue GetValue(const std::string & _Identifier) const override;

  bool SetValue(const std::string & _Identifier, const TPropertyValue & _Value) override;

  void VisitAssetReferences(const TAssetReferenceVisitor & _Visitor) override;

protected: // CSceneNode

  void OnWriteData(TJson & _Data) const override;

  bool OnReadData(const TJson & _Data, std::string & _Error) override;

public: // Interface

  const TAssetReference & GetMeshRef() const noexcept;

  void SetMeshRef(TAssetReference _MeshRef);

private: // Attributes

  TAssetReference m_MeshRef;
};

} // namespace editor
