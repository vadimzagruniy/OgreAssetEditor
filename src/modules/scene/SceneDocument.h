#pragma once

#include "core/Signal.h"
#include "editor/documents/AssetDocument.h"
#include "model/SceneGraph.h"
#include "model/SceneNodeProperties.h"

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
struct TSceneChange;

class CSceneDocument final:
  public CAssetDocument,
  public std::enable_shared_from_this<CSceneDocument>
{
public: // Types

  using TChangedSignal = core::CSignal<const TSceneChange &>;

public: // Constructor

  explicit CSceneDocument(TAssetRecord _Record);

public: // CAssetDocument

  std::vector<TAssetProperty> GetProperties() const override;

protected: // CAssetDocument

  bool OnSave(std::string & _Error) override;

public: // Interface

  bool Load(const std::string & _Text, std::string & _Error);

  const CSceneGraph & GetGraph() const noexcept;

  std::unordered_set<std::string> TakeDirtyNodes();

  bool CanRename(const std::string & _Name, const std::string & _NewName, std::string & _Error) const;

  bool CanMove(const std::string & _Name, const std::string & _NewParentName) const;

  void CreateNode(const std::string & _ParentName, const std::string & _BaseName, const TSceneNodeProperties & _Properties);

  void PasteSubtree(const TSceneSubtree & _Subtree, const std::string & _ParentName, std::size_t _Index);

  void DuplicateNode(const std::string & _Name);

  void DeleteNode(const std::string & _Name);

  void MoveNode(const std::string & _Name, const std::string & _NewParentName);

  void RenameNode(const std::string & _Name, const std::string & _NewName);

  void PreviewProperties(const std::string & _Name, const TSceneNodeProperties & _Properties, const std::string & _CommandName);

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
    TSceneNodeProperties Before;
  };

private: // Implementation

  void ExecuteCommand(std::string _CommandName, TEdit _Execute, TEdit _Undo);

  void InsertSubtree(std::string _CommandName, const TSceneSubtree & _Subtree, const std::string & _ParentName, std::size_t _Index);

  void Notify(ESceneChangeKind _Kind, const std::string & _Name, const std::string & _NewName = {}) const;

private: // Attributes

  CSceneGraph             m_Graph;
  std::optional<TPreview> m_Preview;
  TChangedSignal          m_ChangedSignal;
};

} // namespace editor
