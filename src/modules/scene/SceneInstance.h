#pragma once

#include "editor/workspaces/AssetInstance.h"
#include "model/SceneNode.h"

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace editor
{

class CSceneInstance final:
  public CAssetInstance
{
public: // Constructor/Destructor

  explicit CSceneInstance(TAssetRecord _Record);

  ~CSceneInstance() override;

public: // CAssetInstance

  std::vector<TAssetProperty> GetProperties() const override;

public: // Interface

  bool HasNode(const std::string & _Name) const;

  const TSceneNode & GetNode(const std::string & _Name) const;

  const std::vector<std::string> & GetRootNodes() const noexcept;

  const std::vector<std::string> & GetChildren(const std::string & _ParentName) const;

  std::string MakeUniqueName(const std::string & _BaseName) const;

  bool InsertNode(const std::string & _Name, const std::string & _ParentName, std::size_t _Index, TSceneNodeProperties _Properties, std::string & _Error);

  bool RemoveNode(const std::string & _Name, std::string & _Error);

  bool MoveNode(const std::string & _Name, const std::string & _NewParentName, std::size_t _Index, std::string & _Error);

  bool RenameNode(const std::string & _Name, const std::string & _NewName, std::string & _Error);

  bool SetProperties(const std::string & _Name, TSceneNodeProperties _Properties, std::string & _Error);

  bool Save(std::string & _Error) const;

private: // Implementation

  std::vector<std::string> & GetSiblings(const std::string & _ParentName);

  bool CheckNewName(const std::string & _Name, std::string & _Error) const;

  bool CheckParent(const std::string & _ParentName, std::string & _Error) const;

  bool IsSameOrDescendant(const std::string & _Name, const std::string & _Ancestor) const;

private: // Attributes

  std::unordered_map<std::string, TSceneNode> m_Nodes;
  std::vector<std::string>                    m_RootNodes;
};

} // namespace editor
