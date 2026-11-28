#include "AssetDatabase.h"
#include "AssetNode.h"
#include "AssetReference.h"
#include "AssetResolveResult.h"

#include <utility>

namespace editor
{

//
// Constructor/Destructor
//

CAssetDatabase::CAssetDatabase():
  m_Revision(0)
{
  // Empty
}

CAssetDatabase::~CAssetDatabase() = default;

//
// Interface
//

void CAssetDatabase::Rebuild(const TAssetNode & _Root)
{
  m_Records.clear();
  m_IdsByPath.clear();

  AddNode(_Root);

  ++m_Revision;
}

std::uint64_t CAssetDatabase::GetRevision() const noexcept
{
  return m_Revision;
}

const std::map<TAssetId, TAssetRecord> & CAssetDatabase::GetRecords() const noexcept
{
  return m_Records;
}

bool CAssetDatabase::FindById(const TAssetId & _Id, TAssetRecord & _Record) const
{
  const auto Record = m_Records.find(_Id);
  if (Record == m_Records.end())
    return false;

  _Record = Record->second;

  return true;
}

bool CAssetDatabase::FindByPath(const std::filesystem::path & _RelativePath, TAssetRecord & _Record) const
{
  const auto Id = m_IdsByPath.find(_RelativePath.lexically_normal());

  return Id != m_IdsByPath.end() && FindById(Id->second, _Record);
}

EAssetResolveResult CAssetDatabase::Resolve(const TAssetReference & _AssetRef, TAssetRecord & _Record) const
{
  if (_AssetRef.Id.IsValid() && FindById(_AssetRef.Id, _Record))
    return EAssetResolveResult::Resolved;

  if (!_AssetRef.PathHint.empty() && FindByPath(std::filesystem::path(_AssetRef.PathHint), _Record))
    return EAssetResolveResult::ResolvedByPath;

  return EAssetResolveResult::Missing;
}

bool CAssetDatabase::MakeReference(const std::filesystem::path & _RelativePath, TAssetReference & _AssetRef) const
{
  TAssetRecord Record;
  if (!FindByPath(_RelativePath, Record))
    return false;

  _AssetRef = TAssetReference{Record.Id, Record.RelativePath.generic_string()};

  return true;
}

bool CAssetDatabase::RefreshReference(TAssetReference & _AssetRef) const
{
  TAssetRecord Record;
  if (Resolve(_AssetRef, Record) == EAssetResolveResult::Missing)
    return false;

  TAssetReference RefreshedAssetRef{Record.Id, Record.RelativePath.generic_string()};
  if (RefreshedAssetRef == _AssetRef)
    return false;

  _AssetRef = std::move(RefreshedAssetRef);

  return true;
}

//
// Implementation
//

void CAssetDatabase::AddNode(const TAssetNode & _Node)
{
  if (!_Node.IsDirectory && _Node.Record.Id.IsValid())
  {
    m_Records[_Node.Record.Id]                                = _Node.Record;
    m_IdsByPath[_Node.Record.RelativePath.lexically_normal()] = _Node.Record.Id;
  }

  for (const auto & Child : _Node.Children)
    AddNode(Child);
}

} // namespace editor
