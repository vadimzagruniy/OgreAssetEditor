#pragma once

#include "TextChange.h"
#include "core/Command.h"

#include <string>

namespace editor
{

class CTextInstance;

class CTextEditCommand final:
  public core::ICommand
{
public: // Constructor

  CTextEditCommand(CTextInstance & _Instance, TTextChange _Change, unsigned int _EditSession);

public: // ICommand

  std::string GetName() const override;

  void Execute() override;

  void Undo() override;

  bool MergeWith(const core::ICommand & _Next) override;

private: // Implementation

  bool IsInsertion() const noexcept;

  bool IsDeletion() const noexcept;

  bool HasLineBreak() const noexcept;

  bool MergeInsertion(const CTextEditCommand & _Next);

  bool MergeDeletion(const CTextEditCommand & _Next);

private: // Attributes

  CTextInstance & m_Instance;
  TTextChange     m_Change;
  unsigned int    m_EditSession;
};

} // namespace editor
