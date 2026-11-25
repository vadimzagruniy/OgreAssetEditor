#pragma once

#include "core/Signal.h"
#include "editor/documents/AssetDocument.h"
#include "editor/inspector/PropertyValue.h"
#include "model/SceneAnimation.h"
#include "model/SceneGraph.h"
#include "model/SceneNodeSnapshot.h"

#include <OgrePrerequisites.h>

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

namespace editor
{

enum class ESceneChangeKind;
class CSceneNodeTypes;
struct TSceneChange;

class CSceneDocument final:
  public CAssetDocument,
  public std::enable_shared_from_this<CSceneDocument>
{
public: // Types

  using TChangedSignal = core::CSignal<const TSceneChange &>;

public: // Constructor

  CSceneDocument(TAssetRecord _Record, std::shared_ptr<const CSceneNodeTypes> _NodeTypes);

public: // CAssetDocument

  std::vector<TAssetProperty> GetProperties() const override;

protected: // CAssetDocument

  bool OnSave(std::string & _Error) override;

public: // Interface

  bool Load(const std::string & _Text, std::string & _Error);

  const CSceneGraph & GetGraph() const noexcept;

  const std::vector<TSceneAnimation> & GetAnimations() const noexcept;

  bool HasAnimation(const std::string & _Path) const;

  std::string ResolveSlot(const std::string & _Path, const std::string & _Slot) const;

  std::string FindSlot(const std::string & _Path, const std::string & _NodeName) const;

  std::unordered_set<std::string> TakeDirtyNodes();

  bool CanRename(const std::string & _Name, const std::string & _NewName, std::string & _Error) const;

  bool CanMove(const std::string & _Name, const std::string & _NewParentName) const;

  void CreateNode(const std::string & _ParentName, const std::string & _BaseName, const TSceneNodeSnapshot & _Snapshot);

  void PasteSubtree(const TSceneSubtree & _Subtree, const std::string & _ParentName, std::size_t _Index);

  void DuplicateNode(const std::string & _Name);

  void DeleteNode(const std::string & _Name);

  void MoveNode(const std::string & _Name, const std::string & _NewParentName);

  void RenameNode(const std::string & _Name, const std::string & _NewName);

  void AddAnimation(const std::string & _Path);

  void RemoveAnimation(const std::string & _Path);

  void BindSlot(const std::string & _Path, const std::string & _Slot, const std::string & _NodeName);

  void PreviewValue(const std::string & _Name, const std::string & _Identifier, const TPropertyValue & _Value, const std::string & _CommandName);

  void PreviewWorldTransform(const std::string & _Name, const Ogre::Affine3 & _World);

  void CommitPreview();

  void CancelPreview();

public: // Signals

  TChangedSignal & GetChangedSignal() noexcept;

private: // Types

  using TEdit = std::function<void(CSceneDocument &)>;

  struct TPreview
  {
    std::string          Name;
    std::string          CommandName;
    TSceneNodeSnapshot   Before;
  };

private: // Implementation

  void ExecuteCommand(std::string _CommandName, TEdit _Execute, TEdit _Undo);

  bool BeginPreview(const std::string & _Name, const std::string & _CommandName);

  void InsertSubtree(std::string _CommandName, const TSceneSubtree & _Subtree, const std::string & _ParentName, std::size_t _Index);

  void Notify(ESceneChangeKind _Kind, const std::string & _Name, const std::string & _NewName = {}) const;

  void ReplaceAnimations(std::string _CommandName, std::vector<TSceneAnimation> _Animations);

  std::vector<TSceneAnimation>::const_iterator FindAnimation(const std::string & _Path) const;

private: // Attributes

  CSceneGraph                  m_Graph;
  std::vector<TSceneAnimation> m_Animations;
  std::optional<TPreview>      m_Preview;
  TChangedSignal               m_ChangedSignal;
};

} // namespace editor
