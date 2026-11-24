#pragma once

#include "SceneTransform.h"
#include "editor/inspector/PropertyValue.h"
#include "modules/scene/SceneJson.h"

#include <string>
#include <string_view>
#include <vector>

namespace editor
{

struct TPropertyDesc;
struct TSceneNodeSnapshot;

class CSceneNode
{
public: // Constants

  static constexpr std::string_view TypeId = "node";

public: // Constructor/Destructor

  CSceneNode();

  virtual ~CSceneNode();

  CSceneNode(const CSceneNode &) = delete;

  CSceneNode & operator=(const CSceneNode &) = delete;

public: // Interface

  const std::string & GetName() const noexcept;

  const std::string & GetParentName() const noexcept;

  const std::vector<std::string> & GetChildren() const noexcept;

  const TSceneTransform & GetTransform() const noexcept;

  void SetTransform(const TSceneTransform & _Transform) noexcept;

  virtual std::string_view GetTypeId() const noexcept;

  TSceneNodeSnapshot CaptureSnapshot() const;

  bool ApplySnapshot(const TSceneNodeSnapshot & _Snapshot, std::string & _Error);

  virtual void DescribeProperties(std::vector<TPropertyDesc> & _Properties) const;

  virtual TPropertyValue GetValue(const std::string & _Identifier) const;

  virtual bool SetValue(const std::string & _Identifier, const TPropertyValue & _Value);

protected: // Events

  virtual void OnWriteData(TSceneJson & _Data) const;

  virtual bool OnReadData(const TSceneJson & _Data, std::string & _Error);

private: // Friends

  friend class CSceneGraph;

private: // Attributes

  std::string              m_Name;
  std::string              m_ParentName;
  std::vector<std::string> m_Children;
  TSceneTransform          m_Transform;
};

} // namespace editor
