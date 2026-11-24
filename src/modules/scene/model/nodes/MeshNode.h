#pragma once

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

protected: // CSceneNode

  void OnWriteData(TSceneJson & _Data) const override;

  bool OnReadData(const TSceneJson & _Data, std::string & _Error) override;

public: // Interface

  const std::string & GetMeshName() const noexcept;

  void SetMeshName(std::string _MeshName);

private: // Attributes

  std::string m_MeshName;
};

} // namespace editor
