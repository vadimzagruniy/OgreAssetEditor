#pragma once

#include "editor/workspaces/AssetInstance.h"
#include "model/SceneNode.h"
#include "model/SceneSubtree.h"

#include <OgreMatrix4.h>
#include <OgrePrerequisites.h>
#include <OgreSharedPtr.h>

#include <cstddef>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace editor
{

class CSceneInstance final:
  public CAssetInstance
{
public: // Constructor/Destructor

  CSceneInstance(TAssetRecord _Record, std::string _ResourceGroup);

  ~CSceneInstance() override;

  CSceneInstance(const CSceneInstance &) = delete;

  CSceneInstance & operator=(const CSceneInstance &) = delete;

public: // CAssetInstance

  std::vector<TAssetProperty> GetProperties() const override;

public: // Interface

  bool HasNode(const std::string & _Name) const;

  const TSceneNode & GetNode(const std::string & _Name) const;

  const std::vector<std::string> & GetRootNodes() const noexcept;

  const std::vector<std::string> & GetChildren(const std::string & _ParentName) const;

  std::string MakeUniqueName(const std::string & _BaseName) const;

  std::size_t GetSiblingIndex(const std::string & _Name) const;

  bool IsSameOrDescendant(const std::string & _Name, const std::string & _Ancestor) const;

  Ogre::Affine3 GetWorldTransform(const std::string & _Name) const;

  TSceneSubtree CaptureSubtree(const std::string & _Name) const;

  bool InsertSubtree(const TSceneSubtree & _Subtree, std::string & _Error);

  bool RemoveSubtree(const std::string & _Name, std::string & _Error);

  bool InsertNode(const std::string & _Name, const std::string & _ParentName, std::size_t _Index, TSceneNodeProperties _Properties, std::string & _Error);

  bool RemoveNode(const std::string & _Name, std::string & _Error);

  bool MoveNode(const std::string & _Name, const std::string & _NewParentName, std::size_t _Index, std::string & _Error);

  bool RenameNode(const std::string & _Name, const std::string & _NewName, std::string & _Error);

  bool SetProperties(const std::string & _Name, TSceneNodeProperties _Properties, std::string & _Error);

  bool Save(std::string & _Error) const;

  bool AcquireMesh(const std::string & _MeshName, Ogre::MeshPtr & _Mesh, std::string & _Error);

  std::unordered_set<std::string> TakeDirtyNodes();

private: // Implementation

  void MarkDirty(const std::string & _Name);

  std::vector<std::string> & GetSiblings(const std::string & _ParentName);

  bool CheckNewName(const std::string & _Name, std::string & _Error) const;

  bool CheckParent(const std::string & _ParentName, std::string & _Error) const;

  void CaptureNode(const std::string & _Name, TSceneSubtree & _Subtree) const;

private: // Attributes

  std::string                                    m_ResourceGroup;
  std::unordered_map<std::string, TSceneNode>    m_Nodes;
  std::vector<std::string>                       m_RootNodes;
  std::unordered_set<std::string>                m_DirtyNodes;
  std::unordered_map<std::string, Ogre::MeshPtr> m_Meshes;
};

} // namespace editor
