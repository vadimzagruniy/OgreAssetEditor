#pragma once

#include "PrimitiveShape.h"
#include "modules/scene/model/SceneNode.h"

#include <string>
#include <string_view>
#include <vector>

namespace editor
{

class CPrimitiveNode final:
  public CSceneNode
{
public: // Constants

  static constexpr std::string_view TypeId = "primitive";

public: // Constructor

  CPrimitiveNode();

public: // CSceneNode

  std::string_view GetTypeId() const noexcept override;

  void DescribeProperties(std::vector<TPropertyDesc> & _Properties) const override;

  TPropertyValue GetValue(const std::string & _Identifier) const override;

  bool SetValue(const std::string & _Identifier, const TPropertyValue & _Value) override;

protected: // CSceneNode

  void OnWriteData(TSceneJson & _Data) const override;

  bool OnReadData(const TSceneJson & _Data, std::string & _Error) override;

public: // Interface

  EPrimitiveShape GetShape() const noexcept;

  void SetShape(EPrimitiveShape _Shape) noexcept;

private: // Attributes

  EPrimitiveShape m_Shape;
};

} // namespace editor
