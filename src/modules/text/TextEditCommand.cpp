#include "TextEditCommand.h"
#include "TextInstance.h"

#include <cctype>
#include <utility>

namespace editor
{

namespace
{

bool IsBlank(char _Character)
{
  return std::isspace(static_cast<unsigned char>(_Character)) != 0;
}

} // namespace

//
// Constructor
//

CTextEditCommand::CTextEditCommand(CTextInstance & _Instance, TTextChange _Change, unsigned int _EditSession):
  m_Instance(_Instance),
  m_Change(std::move(_Change)),
  m_EditSession(_EditSession)
{
  // Empty
}

//
// ICommand
//

std::string CTextEditCommand::GetName() const
{
  if (IsInsertion())
    return "Typing";

  if (IsDeletion())
    return "Delete Text";

  return "Replace Text";
}

void CTextEditCommand::Execute()
{
  m_Instance.Replace(m_Change.Offset, m_Change.RemovedText.size(), m_Change.InsertedText);
}

void CTextEditCommand::Undo()
{
  m_Instance.Replace(m_Change.Offset, m_Change.InsertedText.size(), m_Change.RemovedText);
}

bool CTextEditCommand::MergeWith(const core::ICommand & _Next)
{
  const auto * Next = dynamic_cast<const CTextEditCommand *>(&_Next);
  if (!Next || &Next->m_Instance != &m_Instance || Next->m_EditSession != m_EditSession)
    return false;

  if (HasLineBreak() || Next->HasLineBreak())
    return false;

  return MergeInsertion(*Next) || MergeDeletion(*Next);
}

//
// Implementation
//

bool CTextEditCommand::IsInsertion() const noexcept
{
  return m_Change.RemovedText.empty() && !m_Change.InsertedText.empty();
}

bool CTextEditCommand::IsDeletion() const noexcept
{
  return !m_Change.RemovedText.empty() && m_Change.InsertedText.empty();
}

bool CTextEditCommand::HasLineBreak() const noexcept
{
  return m_Change.InsertedText.find('\n') != std::string::npos;
}

bool CTextEditCommand::MergeInsertion(const CTextEditCommand & _Next)
{
  if (!_Next.IsInsertion() || m_Change.InsertedText.empty())
    return false;

  if (_Next.m_Change.Offset != m_Change.Offset + m_Change.InsertedText.size())
    return false;

  // A blank typed after a word starts the next undo step.
  if (!IsBlank(m_Change.InsertedText.back()) && IsBlank(_Next.m_Change.InsertedText.front()))
    return false;

  m_Change.InsertedText += _Next.m_Change.InsertedText;

  return true;
}

bool CTextEditCommand::MergeDeletion(const CTextEditCommand & _Next)
{
  if (!IsDeletion() || !_Next.IsDeletion())
    return false;

  const auto & NextChange = _Next.m_Change;

  // Backspace removes text in front of the previous deletion, Delete removes it at the same offset.
  if (NextChange.Offset + NextChange.RemovedText.size() == m_Change.Offset)
  {
    m_Change.Offset      = NextChange.Offset;
    m_Change.RemovedText = NextChange.RemovedText + m_Change.RemovedText;
    return true;
  }

  if (NextChange.Offset == m_Change.Offset)
  {
    m_Change.RemovedText += NextChange.RemovedText;
    return true;
  }

  return false;
}

} // namespace editor
