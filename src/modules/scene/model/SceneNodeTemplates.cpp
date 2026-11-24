#include "SceneNodeTemplates.h"
#include "SceneNode.h"

#include <algorithm>
#include <cassert>
#include <utility>

namespace editor
{

namespace
{

std::string MakeBaseName(const std::string & _Label)
{
  std::string BaseName = _Label;
  std::erase(BaseName, ' ');

  return BaseName;
}

} // namespace

//
// Interface
//

void CSceneNodeTemplates::Register(std::string _Identifier, std::string _Label, std::string _Category, const CSceneNode & _Prototype)
{
  assert(!Find(_Identifier) && "Scene node template already registered");

  std::string BaseName = MakeBaseName(_Label);

  m_Templates.push_back({std::move(_Identifier), std::move(_Label), std::move(_Category), std::move(BaseName), _Prototype.CaptureSnapshot()});
}

std::optional<TSceneNodeTemplate> CSceneNodeTemplates::Find(const std::string & _Identifier) const
{
  const auto TemplateIterator = std::find_if(m_Templates.begin(), m_Templates.end(), [&_Identifier](const TSceneNodeTemplate & _Template)
  {
    return _Template.Identifier == _Identifier;
  });

  if (TemplateIterator == m_Templates.end())
    return std::nullopt;

  return *TemplateIterator;
}

const std::vector<TSceneNodeTemplate> & CSceneNodeTemplates::GetTemplates() const noexcept
{
  return m_Templates;
}

} // namespace editor
